#include "stdafx.h"
#include "AudioMan.hpp"
#include "2-application/OS.h"
#include "r2/Im.hpp"

#include <fmod_errors.h>
#include "JSerialize.hpp"
#include "EASTL/map.h"

using namespace std;
using namespace r;
using namespace rd;
using namespace FMOD;
using namespace FMOD::Studio;

AudioMan * AudioMan::me = nullptr;


void FilePlayer::load(const char* name) {
	auto it = files.find(name);
	if( it != files.end() ) return;
	
	Sound* & p = files[name];
	p = nullptr;
	std::string fname = (std::string("res/") + name);
	auto res = am->getCoreSystem()->createSound(fname.c_str(), FMOD_CREATESAMPLE | FMOD_2D, nullptr, &p);
	if(FMOD_OK != res )
		printf("FMOD:AM:Err - file %s - %s\n", fname.c_str(), FMOD_ErrorString(res));
}

void FilePlayer::play(const char* name, float vol) {
	auto it = files.find(name);
	if (it == files.end()) load(name);
	it = files.find(name);

	if (it != files.end()) {
		Channel* ch = 0;
		am->getCoreSystem()->playSound(it->second, nullptr, false, &ch);
		if (ch) {
			auto res = ch->setVolume(vol);
			int here = 0;
		}
	}
}


bool FMOD_CHECK(FMOD_RESULT res) {
#ifdef _DEBUG
	if (res != FMOD_OK) {
		printf("FMOD:AM:Err - %s\n", FMOD_ErrorString(res));
	}
#endif
	return res == FMOD_OK;
}

void AudioMan::setListenerPos(const Vector3& v) {
	if (!allow3dListener)
		return;

	listenerPos = v;
	syncListener();
}

AudioMan::AudioMan() {
	confs = {
		//			fwd						up
		AudioManConf(r::Vector3(0,0,1.0f),r::Vector3(0,-1.0f,0)),//AMCS_PIXELS
		AudioManConf(r::Vector3(-0.707f,-0.707f,0),r::Vector3(0,0,1.0f)),//AMCS_ISO
	};
	conf = confs[(u8)AudioManConfStyle::AMCS_PIXELS];
	fp.am = this;
}

void AudioMan::setConfStyle(AudioManConfStyle acs) {
	conf = confs[(u8)(confStyle = acs)];
	listenerUp = conf.UP;
	listenerFwd = conf.FWD;
	syncListener();
}

AudioMan& AudioMan::get(){
	if (me == nullptr)
		me = new AudioMan();
	return *me;
}

void AudioMan::init(){
	if (initialized)
		return;

	FMOD::Studio::System::create(&fmodSystem);

	int flags = FMOD_STUDIO_INIT_LIVEUPDATE;
#ifdef PASTA_NX
	flags = 0;
#endif

#ifdef PASTA_FINAL
	flags = 0;
#endif

	fmodSystem->initialize(1024, flags, FMOD_INIT_NORMAL, NULL);

	bool loadDefaultMasters = true;

	if (loadDefaultMasters) {
		fmodSystem->loadBankFile(Pasta::FileMgr::getSingleton()->convertResourcePath("fmod/Master.bank").c_str(), FMOD_STUDIO_LOAD_BANK_NORMAL, &masterBank);
		fmodSystem->loadBankFile(Pasta::FileMgr::getSingleton()->convertResourcePath("fmod/Debug.bank").c_str(), FMOD_STUDIO_LOAD_BANK_NORMAL, &masterBank);

		FMOD::Studio::Bank* stringsBank = NULL;
		fmodSystem->loadBankFile(Pasta::FileMgr::getSingleton()->convertResourcePath("fmod/Master.strings.bank").c_str(), FMOD_STUDIO_LOAD_BANK_NORMAL, &stringsBank);

		banks["fmod/Master.bank"] = masterBank;
		banks["fmod/Master.strings.bank"] = stringsBank;
#ifdef _DEBUG
		cout << "Master Bank loaded" << endl;
#endif
	}
	initialized = true;

	auto aa = new AnonAgent([this](double dt) {
		fmodSystem->update();
	});
	aa->name = "Sound Man";
	rs::Svc::reg(aa);
}

void AudioMan::prefetch(const std::string & bnkName) {
	if (rs::Std::exists(banks, string(bnkName)))
		return;

	FMOD::Studio::Bank* bnk = 0;
	FMOD::Studio::Bank* strBnk = 0;
	
	fmodSystem->loadBankFile(Pasta::FileMgr::getSingleton()->convertResourcePath(bnkName).c_str(), FMOD_STUDIO_LOAD_BANK_NONBLOCKING, &bnk);

	std::string strBnkName = rd::String::replace(bnkName, ".bank", ".string.bank");
	fmodSystem->loadBankFile(Pasta::FileMgr::getSingleton()->convertResourcePath(strBnkName).c_str(), FMOD_STUDIO_LOAD_BANK_NONBLOCKING, &strBnk);

	banks[bnkName] = bnk;
	banks[strBnkName] = strBnk;
}

void AudioMan::destroy(){
	banks.clear();
	fmodSystem->release();
}

EventInstance* AudioMan::playOnce(const char* event) {
	if (!enableAudioProcessing) return nullptr;
	EventDescription* ed = 0;
	FMOD_RESULT res = fmodSystem->getEvent(event, &ed); 
	
	FMOD_CHECK(res);

	Str e(event);
	rs::Std::ringBufferPush(eventLog, 16, e);

	if (res != FMOD_OK) 
		return nullptr;

	EventInstance* ei = 0;
	res = ed->createInstance(&ei); FMOD_CHECK(res);
	res = ei->start(); FMOD_CHECK(res);
	res = ei->release(); FMOD_CHECK(res);

	return ei;
}

EventInstance* AudioMan::playOnceSilentErrors(const char* event){
	if (!enableAudioProcessing) return nullptr;
	EventDescription* ed = 0;
	FMOD_RESULT res = fmodSystem->getEvent(event, &ed);
	if (res != FMOD_OK)
		return nullptr;
	EventInstance* ei = 0;
	res = ed->createInstance(&ei); 
	res = ei->start();
	res = ei->release(); 
	return ei;
}

EventInstance* AudioMan::playOnce(const std::string& event, const std::unordered_map<std::string, float> params) {
	if (!enableAudioProcessing) return nullptr;
	EventDescription* ed = 0;
	FMOD_RESULT res = fmodSystem->getEvent(event.c_str(), &ed); FMOD_CHECK(res);
	EventInstance* ei = 0;
	res = ed->createInstance(&ei); FMOD_CHECK(res);
	for (auto& p : params) {
		res = ei->setParameterByName(p.first.c_str(), p.second);
		FMOD_CHECK(res);
	}
	res = ei->start(); FMOD_CHECK(res);
	res = ei->release(); FMOD_CHECK(res);
	return ei;
}

bool rd::AudioMan::playFile(const char* fname, float vol){
	fp.play(fname, vol);
	return false;
}

EventInstance* rd::AudioMan::play(const std::string& event){
	if (!enableAudioProcessing) return nullptr;
	EventDescription* ed = 0;
	FMOD_RESULT res = fmodSystem->getEvent(event.c_str(), &ed); FMOD_CHECK(res);
	EventInstance* ei = 0;
	res = ed->createInstance(&ei); FMOD_CHECK(res);
	res = ei->start(); FMOD_CHECK(res);

	Str e(event);
	rs::Std::ringBufferPush(eventLog, 16, e);
	return ei;
}

EventInstance* AudioMan::playOnce(const std::string & event){
	return playOnce(event.c_str());
}

void AudioMan::stopAll(const std::string& event){
	stopAll(event.c_str());
}

FMOD::Studio::EventDescription* AudioMan::getEvent(const char * ev){
	EventDescription* ed = 0;
	fmodSystem->getEvent(ev, &ed);
	return ed;
}

void AudioMan::getEvent(const string& ev, FMOD::Studio::EventDescription *& desc){
	if (!fmodSystem) return;
	fmodSystem->getEvent(ev.c_str(), &desc);
}

void AudioMan::getEvent(const char* ev, FMOD::Studio::EventDescription*& desc) {
	if (!fmodSystem) return;
	fmodSystem->getEvent(ev, &desc);
}

eastl::vector<string> AudioMan::getEventList(const char* _bnk){
	eastl::vector<string> eventList;
	auto& bnk = banks[_bnk];
	int cnt = 0;
	bnk->getEventCount(&cnt);
	eastl::vector<Studio::EventDescription*> res;
	res.resize(cnt);
	bnk->getEventList(res.data(), cnt, &cnt);
	int mpath = 0;
	eventList.reserve(cnt);
	char path[260] = {};
	for (int i = 0; i < cnt; ++i) {
		path[0] = 0;
		res[i]->getPath(path, 259, &mpath);
		eventList.push_back(string(path));
	}
	return eventList;
}

FMOD::Studio::EventInstance* AudioMan::getEventInstance(const string& ev) {
	return getEventInstance(ev.c_str());
}

FMOD::Studio::EventInstance* AudioMan::getEventInstance(const char* ev){
	EventDescription* ed = 0;
	FMOD_RESULT res = fmodSystem->getEvent(ev, &ed); 
	if (!FMOD_CHECK(res)) {
#ifdef _DEBUG
		cout << "unable to find event " << ev << " please check spelling or loaded banks" << endl;
#endif
		return nullptr;
	}
	EventInstance* ei = 0;
	res = ed->createInstance(&ei);  
	if (!FMOD_CHECK(res)) {
		#ifdef _DEBUG
		cout << "unable to create instance"<< ev << " please check spelling or loaded banks"<< endl;
		#endif
	}
	return ei;
}

void AudioMan::getEventInstance(const char* ev, FMOD::Studio::EventInstance*& evi){
	EventDescription* ed = 0;
	FMOD_RESULT res = fmodSystem->getEvent(ev, &ed); FMOD_CHECK(res);
	res = ed->createInstance(&evi);  FMOD_CHECK(res);
}

FMOD::Studio::EventInstance* AudioMan::getEventInstance(FMOD::Studio::EventDescription*& evDesc) {
	EventInstance* ei = 0;
	FMOD_RESULT res = evDesc->createInstance(&ei);  FMOD_CHECK(res);
	return ei;
}

void AudioMan::getEventInstance(FMOD::Studio::EventDescription*& evDesc, FMOD::Studio::EventInstance*& evi) {
	FMOD_RESULT res = evDesc->createInstance(&evi);  FMOD_CHECK(res);
}

void AudioMan::syncListener() {
	if (allow3dListener) {
		FMOD_3D_ATTRIBUTES attrs = {};

		if(listenerFwd.getNorm())
			listenerFwd.normalize();

		if (listenerUp.getNorm())
			listenerUp.normalize();

		attrs.forward = { listenerFwd.x,listenerFwd.y,listenerFwd.z };
		attrs.up = { listenerUp.x,listenerUp.y,listenerUp.z };
		attrs.position = { listenerPos.x,listenerPos.y,listenerPos.z };
		attrs.velocity = { listenerVel.x,listenerVel.y,listenerVel.z };

		FMOD_CHECK(fmodSystem->setListenerAttributes(0,&attrs));
	}
	//else don't care
}

#ifndef MAX_PATH 
	#define MAX_PATH 260
#endif

FMOD::Studio::Bus* AudioMan::getBus(const char* path) {
	if (path == 0)
		path = "bus:/";
	FMOD::Studio::Bus* b = 0;
	fmodSystem->getBus(path, &b);
	return b;
}

FMOD::Studio::Bus* rd::AudioMan::getMainBus()
{
	return getBus();
}

FMOD::Studio::VCA* AudioMan::getVCA(const char* path) {
	if (path == 0)
		path = "vca:/";
	FMOD::Studio::VCA* v = 0;
	fmodSystem->getVCA(path, &v);
	return v;
}

FMOD::Studio::VCA* rd::AudioMan::getMainVCA()
{
	return getVCA();
}

void AudioMan::im(){
	using namespace ImGui;
	PushID(this);

	char path[MAX_PATH] = {};
	char path2[MAX_PATH] = {};

	if (isMute()) {
		if (ImGui::Button(ICON_MD_VOLUME_MUTE "unmute"))
			setMute(false);
	}
	else {
		if (ImGui::Button(ICON_MD_VOLUME_OFF "mute"))
			setMute(true);
	}

	if (CollapsingHeader("Listener")) {
		Checkbox("allow 3D Listener",&allow3dListener);
		if (allow3dListener) {

			bool changed = false;
			changed |= DragFloat3("pos", listenerPos.ptr(), 0.125, -100.0f, 100.0f);

			changed |= DragFloat3Col(Color::Red,"fwd", listenerFwd.ptr(), 0.125, -100.0f, 100.0f);
			changed |= DragFloat3Col(Color::Blue, "up", listenerUp.ptr(), 0.125, -100.0f, 100.0f);
			changed |= DragFloat3("vel", listenerVel.ptr(), 0.125, -100.0f, 100.0f);

			if (changed)
				syncListener();
			
			if (Button("Reset")) {
				listenerPos = r::Vector3::Zero;
				listenerFwd = conf.FWD;
				listenerUp = conf.UP;
				listenerVel = r::Vector3::Zero;
				syncListener();
			}

			if (Button("Set To Pixel")) 
				setConfStyle(AudioManConfStyle::AMCS_PIXELS);
			if (Button("Set To Iso"))
				setConfStyle(AudioManConfStyle::AMCS_ISO);
		}
		else {

		}
	}

	if (CollapsingHeader("System Parameters")) {
		FMOD_RESULT res;
		int nbGlbPrm = 0;
		res = fmodSystem->getParameterDescriptionCount(&nbGlbPrm); FMOD_CHECK(res);
		vector<FMOD_STUDIO_PARAMETER_DESCRIPTION> params;
		params.resize(nbGlbPrm);
		if (nbGlbPrm) {
			res = fmodSystem->getParameterDescriptionList(params.data(), nbGlbPrm, &nbGlbPrm); FMOD_CHECK(res);
			for (auto& prm : params) {
				Text(prm.name);
			}
		}
	}
	if (CollapsingHeader("Bus List")) {
		auto& bnk = masterBank;
		const int maxBus = 16;
		FMOD::Studio::Bus* buses[maxBus] = {};
		int nbBuses = 0;
		bnk->getBusCount(&nbBuses);
		bnk->getBusList(
			buses,
			maxBus,
			&nbBuses
		);
		
		for (int i = 0; i < nbBuses; ++i) {
			PushID(&buses[i]);
			int npath = 0;
			buses[i]->getPath(path,
				259,
				&npath
			);
			path[npath] = 0;
			float vol = 0;
			buses[i]->getVolume(&vol);
			if (DragFloat(path, &vol, 0.001f, 0.0f, 1.f)) {
				buses[i]->setVolume(vol);
				if( strstr(path,"bus:/"))
					latestUserMasterBusVolume = vol;
			}
			Separator();
			PopID();
		}

		if (!nbBuses) {
			FMOD::Studio::ID	guid = {};
			FMOD::Studio::Bus* mainBus = 0;
			if (FMOD_CHECK(fmodSystem->getBus("bus:/", &mainBus))) {
				Indent();
				if (TreeNode("Main Bus")) {
					float vol = 0;
					mainBus->getVolume(&vol);
					if (DragFloat("vol", &vol, 0.001f, 0.0f, 1.0f)) {
						mainBus->setVolume(vol);
						latestUserMasterBusVolume = vol;
					}
					TreePop();
				}
				Unindent();
			}
		}
	}
	if (CollapsingHeader("VCA List")) {
		auto& bnk = masterBank;
		const int maxVCA = 16;
		FMOD::Studio::VCA* VCAs[maxVCA] = {};
		int nbVCAs = 0;
		bnk->getVCACount(&nbVCAs);
		bnk->getVCAList(
			VCAs,
			maxVCA,
			&nbVCAs
		);

		for (int i = 0; i < nbVCAs; ++i) {
			PushID(&VCAs[i]);
			int npath = 0;
			VCAs[i]->getPath(path,
				259,
				&npath
			);
			path[npath] = 0;
			float vol = 0;
			VCAs[i]->getVolume(&vol);
			if (DragFloat(path, &vol, 0.001f, 0.0f, 1.f)) {
				VCAs[i]->setVolume(vol);
				if (strstr(path, "vca:/"))
					latestUserMasterVCAVolume = vol;
			}
			Separator();
			PopID();
		}

		if (!nbVCAs) {
			FMOD::Studio::ID	guid = {};
			FMOD::Studio::VCA* mainVCA = 0;
			if (FMOD_CHECK(fmodSystem->getVCA("vca:/", &mainVCA))) {
				Indent();
				if (TreeNode("Main VCA")) {
					float vol = 0;
					mainVCA->getVolume(&vol);
					if (DragFloat("vol", &vol, 0.001f, 0.0f, 1.0f)) {
						mainVCA->setVolume(vol);
						latestUserMasterVCAVolume = vol;
					}
					TreePop();
				}
				Unindent();
			}
		}
	}
	if (CollapsingHeader("Event List")) {
		Indent();
		for (auto& p : banks) {
			if (strstr(p.first.c_str(), ".strings")) {
				Selectable(p.first.c_str());
			}
			else if (TreeNodeEx(p.first.c_str(), 0)) {
				PushID(&p);
				FMOD::Studio::Bank* bnk = p.second;
				int cnt = 0;
				bnk->getEventCount(&cnt);
				vector<Studio::EventDescription*> res;
				res.resize(cnt);
				bnk->getEventList(res.data(), cnt, &cnt);

				int mpath = 0;
				int mpath2 = 0;
				// sort res by path
				std::sort(res.begin(), res.end(), [&](Studio::EventDescription* ev1, Studio::EventDescription* ev2) {
					ev1->getPath(path, 259, &mpath);
					ev2->getPath(path2, 259, &mpath2);
					return strcmp(path, path2) == -1;
				});
				
				std::vector<std::pair<std::string, bool>> hierarchy;
				int lastOpen = -1;
				for (int i = 0; i < cnt; ++i) {
					EventDescription* ed = res[i];
					res[i]->getPath(path, 259, &mpath);

					std::string s(path);
					size_t pos = 0;
					int depth = 0;
					std::string token;
					bool fullyOpen = true;
					while ((pos = s.find("/")) != std::string::npos) {
						token = s.substr(0, pos);
						if (hierarchy.size() <= depth) {
							bool open = TreeNodeEx(token.c_str(), 0);
							hierarchy.push_back({ token, open });
							fullyOpen &= open;
						} else if (strcmp(hierarchy[depth].first.c_str(), token.c_str()) != 0) {
							for (int i = depth; i < hierarchy.size(); i++) {
								if(hierarchy[i].second) TreePop();
							}
							hierarchy.resize(depth);
							bool open = TreeNodeEx(token.c_str(), 0);
							hierarchy.push_back({ token, open });
							fullyOpen &= open;
						} else if(!hierarchy[depth].second) {
							fullyOpen &= hierarchy[depth].second;
						}
						s.erase(0, pos + 1);
						depth++;
						if (!fullyOpen) break;
					}
					if (!fullyOpen)
						continue;

					PushID(i);
					bool isActive = intentEvent == path;
					if (isActive) ImGui::PushStyleColor(ImGuiCol_Text, Pasta::Color::Red);
					if (ImGui::Selectable(ICON_MD_SELECT_ALL)) {
						intentEvent = path;
						intentBank = p.first;

						onSelection.trigger();
					}
					SameLine();
					Text(path);
					//SameLine();
					if (Button(ICON_MD_PLAY_ARROW))
						playOnce(path);
					SameLine();
					if (Button(ICON_MD_STOP))
						stopAll(path);
					SameLine();
					if (Button(ICON_FA_CLIPBOARD)) 
						ImGui::SetClipboardText(path);
					SameLine();
					bool is3D = false;
					ed->is3D(&is3D);
					if (is3D) Text("3D");
					else Text("2D");

					if (isActive) ImGui::PopStyleColor();

					Indent();
					FMOD_RESULT param = FMOD_OK;
					int curPrm = 0;
					int maxPrm = 0;
					res[i]->getParameterDescriptionCount(&maxPrm);
					do {
						FMOD_STUDIO_PARAMETER_DESCRIPTION prm;
						param = res[i]->getParameterDescriptionByIndex(
							curPrm,
							&prm
						);
						if (param == FMOD_OK) {
							LabelText("Param Name", "%s", prm.name);
						}
						curPrm++;
					} while (param == FMOD_OK && curPrm < maxPrm);
					Unindent();

					PopID();
				}
				for (int i = 0; i < hierarchy.size(); i++) {
					if (hierarchy[i].second) TreePop();
				}
				PopID();
				TreePop();
			}
		}
		Unindent();
	}

	if (Button("Unload All")) {
		unloadAll();
	}

	if (Button("Add Bank")) {
		string path;
		vector < pair < string, string >> filters;
		if (rs::Sys::filePickForOpen(filters,path)) {
			if (path.find("res") == path.npos) 
				Pasta::OsDialogError("It seems library is not in a res folder.\nCannot read library file", "Error");
			else {
				//Not tested use with caution
				int resPos = path.rfind("res");
				string after = path.substr(resPos + strlen("res/"));
				FMOD::Studio::Bank* bnk = 0;
				FMOD_RESULT res = fmodSystem->loadBankFile(Pasta::FileMgr::getSingleton()->convertResourcePath(after).c_str(), FMOD_STUDIO_LOAD_BANK_NORMAL, &bnk);
				FMOD_CHECK(res);
				banks[after] = bnk;
			}
		}
	}

	if (TreeNode("Event log")) {
		Indent();
		for (auto& s : eventLog)
			ImGui::Text(s);
		Unindent();
		TreePop();
	}
	
	PopID();
}

void AudioMan::unloadAll() {
	for (auto& p : banks) 
		p.second->unload();
	banks.clear();
	masterBank = 0;
}

void AudioMan::serialize(Pasta::JReflect & refl){
	refl.visit(allow3dListener, "allow3dListener");
	refl.visit(listenerPos, "listenerPos");
	refl.visit(listenerFwd, "listenerFwd");
	refl.visit(listenerUp, "listenerUp");
	refl.visit(listenerVel, "listenerVel");

	vector<string> paths;
	if (!refl.isReadMode()) 
		for (auto& p : banks) 
			paths.push_back(p.first);

	refl.visit(paths, "bankPaths");
	if (refl.isReadMode()) {
		for ( auto & path : paths) {
			if (rs::Std::exists(banks, path)) {
				//already loaded let's continue...
				continue;
			}
			FMOD::Studio::Bank* bnk = 0;
			path = rd::String::replace(path, "\\", "/");
			FMOD_RESULT res = fmodSystem->loadBankFile(Pasta::FileMgr::getSingleton()->convertResourcePath(path).c_str(), FMOD_STUDIO_LOAD_BANK_NORMAL, &bnk);
			if (FMOD_CHECK(res)) {
				if ( rd::String::containsI(path.c_str(), "master.bank"))
					masterBank = bnk;
				banks[path] = bnk;
			}
			else {
				trace( string("Error loading : ")+ path);
			}
		}
	}
}

vector<string> AudioMan::getBankList(){
	vector<string>res;
	for (auto& p : banks)
		res.push_back(p.first);
	return res;
}

void rd::AudioMan::unload(const char* bnk){
	banks[bnk]->unloadSampleData();
	banks[bnk]->unload();
	for (auto iter = banks.begin(); iter != banks.end();) {
		if( (*iter).first == bnk)
			iter = banks.erase(iter);
		++iter;
	}
}

void rd::AudioMan::ev_setPos(FMOD::Studio::EventInstance* ev, const Vector3& pos){
	if (!ev) return;
	FMOD_3D_ATTRIBUTES attrs = {};
	attrs.forward = { -listenerFwd.x,-listenerFwd.y,-listenerFwd.z };
	attrs.up = { listenerUp.x,listenerUp.y,listenerUp.z };
	attrs.position = { pos.x,pos.y,pos.z };
	attrs.velocity = { };
	FMOD_CHECK(ev->set3DAttributes(&attrs));
}

FMOD::Studio::Bank* AudioMan::getBank(const char* bnk){
	for (auto& p : banks) 
		if (strstr(p.first.c_str(), bnk))
			return p.second;
	return nullptr;
}

void AudioMan::stopAll(const char* _ev) {
	auto ev = getEvent(_ev);
	if (ev)
		ev->releaseAllInstances();
}

void AudioMan::getParamTypeName(FMOD_STUDIO_PARAMETER_TYPE t,std::string & ret) {
	switch (t)
	{
	case FMOD_STUDIO_PARAMETER_GAME_CONTROLLED: { ret = "GAME_CONTROLLED"; break; }
	case FMOD_STUDIO_PARAMETER_AUTOMATIC_DISTANCE: { ret = "AUTOMATIC_DISTANCE"; break; }
	case FMOD_STUDIO_PARAMETER_AUTOMATIC_EVENT_CONE_ANGLE: { ret = "AUTOMATIC_EVENT_CONE_ANGLE"; break; }
	case FMOD_STUDIO_PARAMETER_AUTOMATIC_EVENT_ORIENTATION: { ret = "AUTOMATIC_EVENT_ORIENTATION"; break; }
	case FMOD_STUDIO_PARAMETER_AUTOMATIC_DIRECTION: { ret = "AUTOMATIC_DIRECTION"; break; }
	case FMOD_STUDIO_PARAMETER_AUTOMATIC_ELEVATION: { ret = "AUTOMATIC_ELEVATION"; break; }
	case FMOD_STUDIO_PARAMETER_AUTOMATIC_LISTENER_ORIENTATION: { ret = "AUTOMATIC_LISTENER_ORIENTATION"; break; }
	case FMOD_STUDIO_PARAMETER_AUTOMATIC_SPEED: { ret = "AUTOMATIC_SPEED"; break; }
	case FMOD_STUDIO_PARAMETER_AUTOMATIC_SPEED_ABSOLUTE: { ret = "AUTOMATIC_SPEED_ABSOLUTE"; break; }
	}
}

void AudioMan::getParamFlagDesc(FMOD_STUDIO_PARAMETER_FLAGS t, std::string& ret) {
	if (t & FMOD_STUDIO_PARAMETER_READONLY)		ret += " readonly";
	if (t & FMOD_STUDIO_PARAMETER_AUTOMATIC)	ret += " auto";
	if (t & FMOD_STUDIO_PARAMETER_GLOBAL)		ret += " global";
	if (t & FMOD_STUDIO_PARAMETER_DISCRETE)		ret += " discrete";
}

bool rd::AudioMan::isMute()
{
	float vol = 1.0f;
	float fvol = 1.0f;
	getMainBus()->getVolume(&vol, &fvol);
	return vol == 0.0f;
}

void rd::AudioMan::setMute(bool onOff){
	auto b = getMainBus();

	if (!b) return;

	if (onOff)
		b->setVolume(0.0);
	else
		b->setVolume(latestUserMasterBusVolume);
}

FMOD::System* rd::AudioMan::getCoreSystem()
{
	FMOD::System* s = nullptr;
	fmodSystem->getCoreSystem(&s);
	return s;
}
