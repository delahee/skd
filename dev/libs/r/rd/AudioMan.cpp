#include "stdafx.h"

#include "2-application/OS.h"

#include "r2/Im.hpp"
#include "JSerialize.hpp"
#include "AudioMan.hpp"
#include "r3/r3Im.hpp"
#include <fmod_errors.h>

using namespace r;
using namespace std;
using namespace rd;
using namespace FMOD;
using namespace FMOD::Studio;

AudioMan * AudioMan::me = nullptr;

rd::EventRef::EventRef(const char* _path, const char* _guid) {
	path = _path;
	FMOD::Studio::parseID(_guid,&guid);
}

rd::EventRef::EventRef(const char* _path, const FMOD_GUID& _guid) {
	path = _path;
	guid = _guid;
}

bool rd::EventRef::hasInstance() const{
	return 0!=inst;
}

bool rd::FilePlayer::hasOnDisk(const char* name){
	return rd::RscLib::has( name );
};

bool rd::FilePlayer::hasInCache(const char* name) {
	auto it = files.find(name);
	return (it != files.end());
};

void FilePlayer::load(const char* name, bool loop) {
	auto it = files.find(name);
	if (it != files.end()) 
		return;
	std::string fname = name;
#ifdef PASTA_NX
	fname = "rom:/" + fname;
#else
	if (!rd::String::startsWith(fname, "res")) // TODO use pasta file path conversion in place of this ugly thing
		fname = "res/" + fname;
#endif
	Sound* & p = files[name];
	p = nullptr;

	auto flags = FMOD_CREATESAMPLE | FMOD_2D;//unpack all to mem, 
	if (loop)
		flags |= FMOD_LOOP_NORMAL;
	auto res = am->getCoreSystem()->createSound(fname.c_str(), flags, nullptr, &p);
	if (FMOD_OK != res)
		printf("FMOD:AM:Err - file %s - %s\n", fname.c_str(), FMOD_ErrorString(res));
	else {
		
	}
}

void FilePlayer::play(const char* name, float vol, bool loop) {
	auto it = files.find(name);
	if (it == files.end()) load(name, loop);
	it = files.find(name);

	if (it != files.end()) {
		Channel* ch = 0;
		am->getCoreSystem()->playSound(it->second, nullptr, false, &ch);
		if (ch) {
			auto res = ch->setVolume(vol);
			int here = 0;
			chns[name] = ch;
		}
	}
}

void rd::FilePlayer::stop(const char* name) {
	auto it = chns.find(name);
	if (it != chns.end()) 
		it->second->stop();
}

void rd::FilePlayer::im(){
	using namespace ImGui;
	if(TreeNode("chn")){
		for(auto& c : chns){
			if( TreeNode(c.first)){
				auto chn = c.second;
				bool pl = false;
				chn->isPlaying(&pl);
				Value("isPlaying",pl);
				TreePop();
			}
		}
		TreePop();
	}
}


bool FMOD_CHECK(FMOD_RESULT res, const char * msg) {
#ifdef _DEBUG
	if (res != FMOD_OK) {
		if( msg)
			printf("FMOD:AM:Err - %s ( %s )\n", FMOD_ErrorString(res), msg );
		else 
			printf("FMOD:AM:Err - %s\n", FMOD_ErrorString(res));
	}
#endif
	return res == FMOD_OK;
}

eastl::vector<FMOD_STUDIO_USER_PROPERTY> getUserProperties(FMOD::Studio::EventDescription* desc){
	if (!desc) return {};

	eastl::vector<FMOD_STUDIO_USER_PROPERTY> res;

	int nb = 0;
	desc->getUserPropertyCount(&nb);
	if(nb){
		for(int i = 0; i < nb;++i){
			FMOD_STUDIO_USER_PROPERTY p;
			desc->getUserPropertyByIndex(i, &p);
			res.push_back(p);
		}
	}

	return res;
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
#ifdef _DEBUG
	//debug = true;
#endif
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

	const char* masterBnk = "fmod/Master.bank";
	const char* masterStrBnk = "fmod/Master.strings.bank";

	fmodSystem->loadBankFile(Pasta::FileMgr::getSingleton()->convertResourcePath(masterBnk).c_str(), FMOD_STUDIO_LOAD_BANK_NORMAL, &masterBank);
	FMOD::Studio::Bank* stringsBank = NULL;
	fmodSystem->loadBankFile(Pasta::FileMgr::getSingleton()->convertResourcePath(masterStrBnk).c_str(), FMOD_STUDIO_LOAD_BANK_NORMAL, &stringsBank);

	banks[masterBnk] = masterBank;
	banks[masterBnk] = stringsBank;
	// UI should be always loaded

	initialized = true;

	auto aa = new AnonAgent([this](double dt) {
		fmodSystem->update();
	});
	aa->name = "Sound Man";
	rs::Svc::reg(aa);
}

void rd::AudioMan::prefetch(const char* _bnk, bool addStrBnk){
	std::string bnkName = _bnk;
	if (rs::Std::exists(banks, bnkName.c_str()))
		return;

	FMOD::Studio::Bank* bnk = 0;
	fmodSystem->loadBankFile(Pasta::FileMgr::getSingleton()->convertResourcePath(bnkName).c_str(), FMOD_STUDIO_LOAD_BANK_NONBLOCKING, &bnk);
	banks[bnkName] = bnk;

	if (addStrBnk) {
		FMOD::Studio::Bank* strBnk = 0;
		std::string strBnkName = rd::String::replace(bnkName, ".bank", ".string.bank");
		fmodSystem->loadBankFile(Pasta::FileMgr::getSingleton()->convertResourcePath(strBnkName).c_str(), FMOD_STUDIO_LOAD_BANK_NONBLOCKING, &strBnk);
		banks[strBnkName] = strBnk;
	}
}

void AudioMan::prefetch(const std::string & bnkName) {
	prefetch(bnkName.c_str());
}

void AudioMan::fullFetch(const char* _bnk) {
	prefetch(_bnk);
	auto res = banks[_bnk]->loadSampleData();
	if (res != FMOD_OK)
		traceWarning(Str128f("%s has not been totally load %s", _bnk, FMOD_ErrorString(res)));
}

void AudioMan::destroy(){
	banks.clear();
	fmodSystem->release();
}

EventInstance* AudioMan::playOnce(const FMOD_GUID & guid) {
	if (!enableAudioProcessing) return nullptr;
	EventDescription* ed = 0;
	FMOD_RESULT res = fmodSystem->getEventByID(&guid, &ed);

	if (logs & LOG_PLAY_EVENT)
		FMOD_CHECK(res);

	if (res != FMOD_OK) {
		rs::Std::ringBufferPush(eventLog, 16, Str(Str(ICON_MD_ERROR) + GUIDtoStr(guid)));
		return nullptr;
	}

	if (logs & LOG_PLAY_EVENT)
		trace(Str128f("[AM] playOnce %s", GUIDtoStr(guid).c_str()));

	EventInstance* ei = 0;
	res = ed->createInstance(&ei); FMOD_CHECK(res);
	res = ei->start(); FMOD_CHECK(res);
	res = ei->release(); FMOD_CHECK(res);

	if( debug )
		rs::Std::ringBufferPush(eventLog, 16, (Str&)GUIDtoStr(guid));

	return ei;
}

EventInstance* AudioMan::playOnce(const rd::EventRef& ref) {
	if (!ref.path.empty()) {
		if (hasEvent(ref.path))
			return ref.inst = playOnce(ref.path.c_str());
		if (logs & LOG_MASK::LOG_PLAY_EVENT)
			traceWarning( Str64f("Event Ref path cannot be played %s", ref.path.c_str()));
	}
	if(!ref.isGUIDEmpty())
		return ref.inst = playOnce(ref.guid);
	return 0;
}

EventInstance* AudioMan::playOnce(const char* event) {
	if (!enableAudioProcessing) return nullptr;
	EventDescription* ed = 0;
	FMOD_RESULT res = fmodSystem->getEvent(event, &ed); 
	
	if(logs & LOG_PLAY_EVENT)
		FMOD_CHECK(res, event);

	if (res != FMOD_OK) {
		if( debug ) rs::Std::ringBufferPush(eventLog, 16, Str(std::string(ICON_MD_ERROR) + event));
		return nullptr;
	}

	if (logs & LOG_PLAY_EVENT)
		trace( Str128f("[AM] playOnce %s", event) );

	EventInstance* ei = 0;
	res = ed->createInstance(&ei); FMOD_CHECK(res);
	res = ei->start(); FMOD_CHECK(res);
	res = ei->release(); FMOD_CHECK(res);

	if (debug)
		rs::Std::ringBufferPush(eventLog, 16, Str(event));

	return ei;
}

EventInstance* AudioMan::playOnceSilentErrors(const char* event){
	if (!enableAudioProcessing) return nullptr;
	EventDescription* ed = 0;
	FMOD_RESULT res = fmodSystem->getEvent(event, &ed);
	if (res != FMOD_OK)
		return nullptr;
	if (logs & LOG_PLAY_EVENT)
		trace( Str128f("[AM] playOnceSilentErrors %s",event));
	EventInstance* ei = 0;
	res = ed->createInstance(&ei); 
	res = ei->start();
	res = ei->release(); 

	if (debug)
		rs::Std::ringBufferPush(eventLog, 16, Str(event));

	return ei;
}

double rd::AudioMan::getDuration(FMOD::Studio::EventDescription& ed) {
	int ms = 0;
	ed.getLength(&ms);
	return ms / 1000.0f;
}

double rd::AudioMan::getDuration(FMOD::Studio::EventInstance& ei){
	EventDescription* ed = 0;
	ei.getDescription(&ed);
	if (ed)
		return getDuration(*ed);
	return 0;
}

EventInstance* AudioMan::playOnce(const std::string& event, const std::unordered_map<std::string, float> &params) {
	if (!enableAudioProcessing) return nullptr;
	EventDescription* ed = 0;
	FMOD_RESULT res = fmodSystem->getEvent(event.c_str(), &ed); FMOD_CHECK(res);
	EventInstance* ei = 0;
	res = ed->createInstance(&ei); FMOD_CHECK(res);

	if (logs & LOG_PLAY_EVENT)
		trace(Str128f("[AM] playOnce %s", event.c_str()));
	for (auto& p : params) {
		res = ei->setParameterByName(p.first.c_str(), p.second);
		FMOD_CHECK(res);
	}
	res = ei->start(); FMOD_CHECK(res);
	res = ei->release(); FMOD_CHECK(res);

	if (debug)
		rs::Std::ringBufferPush(eventLog, 16, Str(event));
	return ei;
}

EventInstance* AudioMan::playOnce(const rd::EventRef& event, const std::unordered_map<std::string, float>& params) {
	if (!enableAudioProcessing) return nullptr;
	EventDescription* ed = 0;
	EventInstance* ei = playOnce(event);
	FMOD_RESULT res = {};
	for (auto& p : params) {
		res = ei->setParameterByName(p.first.c_str(), p.second);
		FMOD_CHECK(res);
	}
	res = ei->start(); FMOD_CHECK(res);
	res = ei->release(); FMOD_CHECK(res);

	if (debug)
		rs::Std::ringBufferPush(eventLog, 16, Str(event.path));
	return ei;
}

bool rd::AudioMan::preloadFile(const char* fname) {
	fp.load(fname);
	return true;
}

bool rd::AudioMan::hasFile(const char* fname){
	return fp.hasInCache(fname) || fp.hasOnDisk(fname);
}

bool rd::AudioMan::playFile(const char* fname, float vol, bool loop){
	fp.play(fname, vol, loop);
	return true;
}

bool rd::AudioMan::isPlaying(const char* evName){
	if (hasEvent(evName))
		return isPlayingEvent(evName);
	else
		return isPlayingFile(evName);
}

bool rd::AudioMan::isPlayingEvent(const char* ev){
	auto evd = getEvent(ev);
	if (!evd) return false;
	const int max = 1;
	EventInstance* evs[max] = {};
	int nb = 0;
	evd->getInstanceList(evs, 16, &nb);
	return nb != 0;
}

bool rd::AudioMan::isPlayingFile(const char* fname){
	auto p = fp.files.find(fname);
	if ( p == fp.files.end())
		return false;

	FMOD::Sound* snd = p->second;
	if (!snd) return false;

	auto it = fp.chns.find(fname);
	if (it != fp.chns.end()) {
		bool r = 0;
		auto res = it->second->isPlaying(&r);
		if(FMOD_CHECK(res))
			return r;
	}
	return false;
}

bool rd::AudioMan::isPlaying(ra::EventInstance* ei){
	if (!ei) return false;
	if (!ei->isValid())
		return false;
	FMOD_STUDIO_PLAYBACK_STATE st= {};
	ei->getPlaybackState(&st);
	return st == FMOD_STUDIO_PLAYBACK_PLAYING || st == FMOD_STUDIO_PLAYBACK_STARTING;
}

bool rd::AudioMan::stopFile(const char* fname) {
	fp.stop(fname);
	return true;
}

EventInstance* rd::AudioMan::play(const FMOD_GUID& guid) {
	if (!enableAudioProcessing) return nullptr;
	EventDescription* ed = 0;
	FMOD_RESULT res = fmodSystem->getEventByID(&guid, &ed); FMOD_CHECK(res);
	EventInstance* ei = 0;
	res = ed->createInstance(&ei);

	Str64 lblGuid = GUIDtoStr(guid);
	if (!FMOD_CHECK(res)) {
		rs::Std::ringBufferPush(eventLog, 16, Str(ICON_MD_ERROR) + lblGuid);
		return 0;
	}
	res = ei->start();
	if (!FMOD_CHECK(res)) {
		rs::Std::ringBufferPush(eventLog, 16, Str(ICON_MD_ERROR) + lblGuid);
		return 0;
	}
	if (logs & LOG_PLAY_EVENT)
		trace(Str128f("[AM] play %s", lblGuid.c_str()));

	if( debug)
		rs::Std::ringBufferPush(eventLog, 16, Str(lblGuid));
	return ei;
}

EventInstance* rd::AudioMan::play(const rd::EventRef& ref) {
	if (hasEvent(ref.path))
		return ref.inst = play(ref.path.c_str());
	else
		return ref.inst = play(ref.guid);
}

EventInstance* rd::AudioMan::play(const char * ev) {
	if (!enableAudioProcessing) 
		return nullptr;
	EventDescription* ed = 0;
	FMOD_RESULT res = fmodSystem->getEvent(ev, &ed); FMOD_CHECK(res, ev);
	EventInstance* ei = 0;
	res = ed->createInstance(&ei);
	if (!FMOD_CHECK(res, ev)) {
		rs::Std::ringBufferPush(eventLog, 16, Str(std::string(ICON_MD_ERROR) + ev));
		return 0;
	}
	res = ei->start();
	if (!FMOD_CHECK(res, ev)) {
		rs::Std::ringBufferPush(eventLog, 16, Str(std::string(ICON_MD_ERROR) + ev));
		return 0;
	}
	if (logs & LOG_PLAY_EVENT)
		trace(Str128f("[AM] play %s", ev));
	Str e(ev);
	rs::Std::ringBufferPush(eventLog, 16, e);
	return ei;
}

EventInstance* rd::AudioMan::play(const std::string& event){
	return play(event.c_str());
}

EventInstance* AudioMan::playOnce(const std::string & event){
	return playOnce(event.c_str());
}

void AudioMan::stopAll(const std::string& event){
	stopAll(event.c_str());
}

FMOD::Studio::EventInstance* rd::AudioMan::getActiveEventInstance(const char* ev){
	if (!hasEvent(ev))
		return 0;
	auto evd = getEvent(ev);
	if (!evd) return 0;
	const int max = 4;
	EventInstance* evs[max] = {};
	int nb = 0;
	evd->getInstanceList(evs, max, &nb);
	if (!nb)return 0;
	return evs[0];
}

void rd::AudioMan::stopGraciously(const FMOD_GUID & ev) {
	auto evd = getEvent(ev);
	if (!evd) return;
	const int max = 16;
	EventInstance* evs[max] = {};
	int nb = 0;
	evd->getInstanceList(evs, max, &nb);
	for (int i = 0; i < nb; ++i) {
		evs[i]->stop(FMOD_STUDIO_STOP_ALLOWFADEOUT);
		evs[i]->release();
	}
}

void rd::AudioMan::stopGraciously(const char *ev){
	auto evd = getEvent(ev);
	if (!evd) return;
	const int max = 16;
	EventInstance* evs[max] = {};
	int nb = 0;
	evd->getInstanceList(evs, max, &nb);
	for (int i = 0; i < nb; ++i) {
		evs[i]->stop(FMOD_STUDIO_STOP_ALLOWFADEOUT);
		evs[i]->release();
	}
}

void rd::AudioMan::stopBrutally(const char* ev){
	auto evd = getEvent(ev);
	if( evd )
		evd->releaseAllInstances();
}

void rd::AudioMan::stopBrutally(const FMOD_GUID& ev){
	auto evd = getEvent(ev);
	if (evd)
		evd->releaseAllInstances();
}

bool AudioMan::hasEvent(const FMOD_GUID& ref) {
	EventDescription* ed = 0;
	FMOD_RESULT fr = fmodSystem->getEventByID(&ref, &ed);
	return fr == FMOD_OK;
}

bool AudioMan::hasBank(rd::EventRef& ev) {
	return getBank(ev.bank.c_str());
}

bool rd::AudioMan::hasInternalLoop(FMOD::Studio::EventDescription * ed){
	bool isOneShot = false;
	ed->isOneshot(&isOneShot);
	return !isOneShot;
}

bool rd::AudioMan::isOneShot(FMOD::Studio::EventDescription* ed){
	bool isOneShot = false;
	ed->isOneshot(&isOneShot);
	return isOneShot;
}

bool AudioMan::hasEvent(rd::EventRef& ev) {
	if (ev.isEmpty() || !fmodSystem) return 0;

	if (!ev.path.empty() && hasEvent(ev.path.c_str()))
		return true;
	else if (hasEvent(ev.guid))
		return true;
	return false;
}

bool AudioMan::hasEvent(const char* ev) {
	if (!ev || !*ev || !fmodSystem) return 0;
	EventDescription* ed = 0;
	FMOD_RESULT fr = fmodSystem->getEvent(ev, &ed);
	return (fr == FMOD_OK) ? ed : 0;
}

FMOD::Studio::EventDescription* AudioMan::getEvent(const rd::EventRef& ref) {
	EventDescription* ed = 0;
	if (hasEvent(ref.path.c_str()))
		return getEvent(ref.path.c_str());
	else
		return getEvent(ref.guid);
}

FMOD::Studio::EventDescription* AudioMan::getEvent(const FMOD_GUID& ref) {
	EventDescription* ed = 0;
	FMOD_RESULT fr = fmodSystem->getEventByID(&ref,&ed);
	return ed;
}

FMOD::Studio::EventDescription* AudioMan::getEvent(const char * ev){
	if (!ev || !*ev || !fmodSystem ) return 0;
	EventDescription* ed = 0;
	FMOD_RESULT fr = fmodSystem->getEvent(ev, &ed);

	if (fr == FMOD_OK) {
		if (rd::Bits::is(logs, LOG_PLAY_EVENT))
			trace(Str128f("[AM] getEvent %s", ev));
	}
	else {
		if (rd::Bits::is(logs, LOG_PLAY_EVENT)) // less chatty for general tools
			traceError(Str128f("[AM] Error getting event %s", ev));
	}
	
	return (fr == FMOD_OK) ? ed : 0;
}

void AudioMan::getEvent(const std::string& ev, FMOD::Studio::EventDescription *& desc){
	if (!fmodSystem) return;
	fmodSystem->getEvent(ev.c_str(), &desc);
}

void AudioMan::getEvent(const char* ev, FMOD::Studio::EventDescription*& desc) {
	if (!fmodSystem) return;
	fmodSystem->getEvent(ev, &desc);
}

eastl::vector<Str> AudioMan::getEventList(const char* _bnk){
	eastl::vector<Str> eventList;
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
		eventList.push_back(Str(path));
	}
	return eventList;
}

FMOD::Studio::EventInstance* AudioMan::getEventInstance(const std::string& ev) {
	return getEventInstance(ev.c_str());
}

FMOD::Studio::EventInstance* AudioMan::getEventInstance(const char* ev){
	EventDescription* ed = 0;
	FMOD_RESULT res = fmodSystem->getEvent(ev, &ed); 
	if (!FMOD_CHECK(res)) {
#ifdef _DEBUG
		std::cout << "unable to find event " << ev << " please check spelling or loaded banks" << std::endl;
#endif
		return nullptr;
	}
	EventInstance* ei = 0;
	res = ed->createInstance(&ei);  
	if (!FMOD_CHECK(res)) {
		#ifdef _DEBUG
		std::cout << "unable to create instance"<< ev << " please check spelling or loaded banks"<< std::endl;
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

FMOD::Studio::Bus* AudioMan::getBus(const char* path) {
	if (!fmodSystem) 
		return 0;

	if (path == 0)
		path = "bus:/";
	FMOD::Studio::Bus* b = 0;
	fmodSystem->getBus(path, &b);
	return b;
}

FMOD::Studio::Bus* rd::AudioMan::getMainBus(){
	return getBus();
}

std::string rd::AudioMan::getVCAName(FMOD::Studio::VCA* vca){
	char p[_MAX_PATH] = {};
	int len = 0;
	vca->getPath(p, _MAX_PATH - 4, &len);
	return std::string(p);
}

FMOD::Studio::VCA* AudioMan::getVCA(const char* path) {
	if (!fmodSystem) 
		return 0;

	if (path == 0)
		path = "vca:/";
	FMOD::Studio::VCA* v = 0;
	fmodSystem->getVCA(path, &v);
	return v;
}

FMOD::Studio::VCA* rd::AudioMan::getMainVCA(){
	return getVCA();
}

bool rd::AudioMan::muteVCA(const char* path){
	auto vca = getVCA(path);
	if (!vca) return false;
	vca->setVolume(0);
	return true;
}

eastl::vector<FMOD::Studio::VCA*> rd::AudioMan::getAllVCA() {
	eastl::vector<FMOD::Studio::VCA*> v;
	auto bnk = getMasterBank();
	
	int nb{};
	auto res = bnk->getVCACount(&nb);
	if (!FMOD_CHECK(res))
		return v;

	v.reserve(nb);
	v.resize(nb);

	int done = 0;
	bnk->getVCAList( v.data(), nb, &done);

	return v;
}

void AudioMan::im(){
	using namespace ImGui;
	PushID(this);

	char path[512] = {};
	char path2[512] = {};

	if (isMute()) {
		if (ImGui::Button(ICON_MD_VOLUME_MUTE "unmute"))
			setMute(false);
	}
	else {
		if (ImGui::Button(ICON_MD_VOLUME_OFF "mute"))
			setMute(true);
	}

	if (ImGui::CollapsingHeader("Debug")) {
		ImGui::Text("Log level: ");
		SameLine();
		bool playFile = logs & LOG_PLAY_FILE;
		ImGui::Checkbox("LOG PLAY FILE", &playFile);
		if (playFile)
			logs |= LOG_PLAY_FILE;
		else
			logs &= ~LOG_PLAY_FILE;
		SameLine();
		bool playEv = logs & LOG_PLAY_EVENT;
		ImGui::Checkbox("LOG PLAY EVENT", &playEv);
		if (playEv)
			logs |= LOG_PLAY_EVENT;
		else
			logs &= ~LOG_PLAY_EVENT;
	}

	if (CollapsingHeader("Listener")) {
		Checkbox("allow 3D Listener",&allow3dListener);
		if (allow3dListener) {

			bool changed = false;

			changed |= DragFloat3("pos", listenerPos.ptr(), 0.125, -100.0f, 100.0f);

			bool showListener = true;
			if (showListener || IsItemHovered()) 
				r3::im::gizmo(listenerPos);

			if (r::Math::isDenormalized(listenerPos.x)) 
				Warning("x is denormalized");
			if (r::Math::isDenormalized(listenerPos.y))
				Warning("y is denormalized");
			if (r::Math::isDenormalized(listenerPos.z))
				Warning("z is denormalized");

			changed |= DragFloat3Col(Color::Orange,"fwd", listenerFwd.ptr(), 0.125, -100.0f, 100.0f);
			changed |= DragFloat3Col(Color::Pink, "up", listenerUp.ptr(), 0.125, -100.0f, 100.0f);
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
		std::vector<FMOD_STUDIO_PARAMETER_DESCRIPTION> params;
		params.resize(nbGlbPrm);
		if (nbGlbPrm) {
			res = fmodSystem->getParameterDescriptionList(params.data(), nbGlbPrm, &nbGlbPrm); FMOD_CHECK(res);
			for (auto& prm : params) {
				ImGui::Text(prm.name);
				ImGui::Value("dflt",prm.defaultvalue);
				float val = 0;
				float fval = 0;
				fmodSystem->getParameterByName(prm.name, &val, &fval);
				ImGui::Value("val",val);
				ImGui::Value("final val",fval);
			}
		}
	}
	if (CollapsingHeader("Buses")) {
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
				512,
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
	if (CollapsingHeader("VCA")) {
		ImGui::PushItemWidth(180);
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
				512,
				&npath
			);
			path[npath] = 0;
			float vol = 0;
			VCAs[i]->getVolume(&vol);
			if (SliderFloat(path, &vol, 0.0f, 1.f)) {
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
		ImGui::PopItemWidth();
	}

	if (CollapsingHeader("Files")) {
		Indent();
		for(auto& p : fp.files){
			ImGui::Text(p.first.c_str());
			PushID(p.first.c_str());
			if( p.second ){
				SameLine();
				if( Button(ICON_MD_PLAY_ARROW))
					fp.play(p.first.c_str());
			}
			else {
				SameLine();
				ImGui::Text("Not found");
			}
			PopID();
		}
		for (auto& p : fp.chns) 
			ImGui::Text(p.first.c_str());
		
		Unindent();
	}

	if (CollapsingHeader("Playing")) {
		Indent();
		for (auto& kv : banks) {
			auto p = [=](FMOD::Studio::EventDescription* ed) ->void{
				int nb = 0;
				ed->getInstanceCount(&nb);
				if(nb){
					std::vector<FMOD::Studio::EventInstance*> res;
					res.resize(nb);
					int nbReal = 0;
					ed->getInstanceList( res.data(), nb, &nbReal);
					if(nbReal){
						for (int i = 0; i < nbReal; ++i) {
							auto el = res[i];
							PushID(el);
							using namespace ImGui;
							char p[260] = { 0 };
							ed->getPath(p,260,0);
							ImGui::Text(p);
							imEventInst(el);
							
							PopID();
						}
					}
				}

			};
			traverseEvents(kv.second, p);
		}
		Unindent();
	}
	

	if (CollapsingHeader("Events")) {
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
				std::vector<Studio::EventDescription*> res;
				res.resize(cnt);
				bnk->getEventList(res.data(), cnt, &cnt);

				int mpath = 0;
				int mpath2 = 0;
				// sort res by path
				std::sort(res.begin(), res.end(), [&](Studio::EventDescription* ev1, Studio::EventDescription* ev2) {
					ev1->getPath(path, 512, &mpath);
					ev2->getPath(path2, 512, &mpath2);
					return strcmp(path, path2) < 0;
				});
				
				std::vector<std::pair<std::string, bool>> hierarchy;
				int lastOpen = -1;
				for (int i = 0; i < cnt; ++i) {
					EventDescription* ed = res[i];
					res[i]->getPath(path, 512, &mpath);

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
					if (Button(ICON_FA_CLIPBOARD "path")) 
						ImGui::SetClipboardText(path);
					SameLine();
					if (Button(ICON_FA_CLIPBOARD "guid")) {
						FMOD_GUID guid = {};
						res[i]->getID(&guid);
						char idStr[512] = {};
						sprintf(idStr,"{%08lX-%04hX-%04hX-%02hhX%02hhX-%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX}",
							guid.Data1, guid.Data2, guid.Data3,
							guid.Data4[0], guid.Data4[1], guid.Data4[2], guid.Data4[3],
							guid.Data4[4], guid.Data4[5], guid.Data4[6], guid.Data4[7]);
						ImGui::SetClipboardText( idStr );
					}
					SameLine();
					bool is3D = false;
					ed->is3D(&is3D);
					if (is3D) Text("3D");
					else Text("2D");

					if (TreeNode("Desc")) {
						imEventDesc(ed);
						TreePop();
					}
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
		for (auto& s : eventLog) {
			ImGui::Text(s);
			if (!hasEvent(s)) {
				SameLine();
				Warning(Str64f("Cannot find %s. Check FMOD Bank.", s.c_str()));
			}
		}
		Unindent();
		TreePop();
	}

	if (TreeNode("Snapshot log")) {
		Indent();
		std::vector<Str> snapshots = { "event:/Game/Snapshot/State_Battle", "event:/Game/Snapshot/State_Explore", "event:/UI/HUD/UI_HUD_Pause", "event:/Game/Snapshot/State_Interlevel" };
		for (int i = 0; i < snapshots.size(); ++i) {
			auto active = isPlaying(snapshots[i].c_str()) ? Str(Str(ICON_MD_PLAY_ARROW) + Str("Playing: ")) : Str(Str(ICON_MD_STOP) + Str("Stopped: "));
			ImGui::Text(active);
			SameLine();
			ImGui::Text(snapshots[i]);
		}
		Unindent();
		TreePop();
	}
	PopID();
}

void rd::AudioMan::imEventInst(FMOD::Studio::EventInstance* inst){
	using namespace ImGui;
	int tmlPos = 0; inst->getTimelinePosition(&tmlPos); Value("timeline pos", tmlPos);
	
	if (TreeNode("Details")) {
		u32 excl = 0, incl = 0;
		inst->getCPUUsage(&excl, &incl);
		Value("cpu excl", excl);
		Value("cpu incl", incl);
		TreePop();
	}

	if (TreeNode("Description")) {
		EventDescription *ed = 0;
		inst->getDescription(&ed);
		imEventDesc(ed);
		TreePop();
	}
}

void rd::AudioMan::imEventDesc(FMOD::Studio::EventDescription* desc){
	using namespace ImGui;
	vec2 dist = {};
	desc->getMinMaxDistance(&dist.x,&dist.y);
	ImGui::PushItemWidth(180);
	DragFloat2("min max", dist.ptr());
	bool os = false;
	desc->isOneshot(&os);
	Value("one shot", os);
	int instCount = 0; desc->getInstanceCount(&instCount); 	Value("inst count", instCount);
	auto props = getUserProperties(desc);
	Text("user Properties : %d", props.size());
	for (auto& p : props) 
		ImGui::Text(p.name);
	ImGui::PopItemWidth();
}

bool rd::AudioMan::hasParameter(FMOD::Studio::EventInstance& ev, const char * name) {
	if (!fmodSystem) return false;
	float v, vv;
	auto res = ev.getParameterByName(name, &v,&vv);
	return FMOD_OK == res;
}

bool rd::AudioMan::hasParameter(const char* prm) {
	if (!fmodSystem) return false;

	int nbGlbPrm = 0;
	fmodSystem->getParameterDescriptionCount(&nbGlbPrm);
	std::vector<FMOD_STUDIO_PARAMETER_DESCRIPTION> params;
	params.resize(nbGlbPrm);
	if (!nbGlbPrm) return false;
	fmodSystem->getParameterDescriptionList(params.data(), nbGlbPrm, &nbGlbPrm);
	for (auto& p : params)
		if (rd::String::equals(prm, p.name))
			return true;
	return false;
}

FMOD_RESULT rd::AudioMan::setParameter(const char* prm, float value) {
	return fmodSystem->setParameterByName(prm, value);
}

FMOD_RESULT rd::AudioMan::setParameter(const char* prm, const char * value) {
	return fmodSystem->setParameterByNameWithLabel(prm, value);
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

	std::vector<std::string> paths;
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

eastl::vector<Str> AudioMan::getBankList(){
	eastl::vector<Str>res;
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

FMOD::Studio::Bank* AudioMan::getMasterBank() {
	return masterBank;
}

FMOD::Studio::Bank* AudioMan::getBank(const char* bnk){
	for (auto& p : banks) 
		if (strstr(p.first.c_str(), bnk))
			return p.second;
	return nullptr;
}

void AudioMan::loadBankIfNot(const char* bnk) {
	if (banks.find(bnk) == banks.end()) {
		trace(Str128f("Load dynamically FMOD Bank %s", bnk));
		prefetch(bnk);
	}
}

void AudioMan::stopAll(const rd::EventRef& ref) {
	if (hasEvent(ref.path))
		stopAll(ref.path.c_str());
	else
		stopGraciously(ref.guid);
}

void AudioMan::stopAll(const char* _ev) {
	stopGraciously(_ev);
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

bool rd::AudioMan::isMute() {
	float vol = 1.0f;
	float fvol = 1.0f;
	getMainBus()->getVolume(&vol, &fvol);
	return vol == 0.0f;
}

void rd::AudioMan::setMute(bool mute) {
	auto b = getMainBus();
	if (!b) return;

	b->setVolume(mute ? 0.0 : latestUserMasterBusVolume);
	onMute(mute);
}

FMOD::System* rd::AudioMan::getCoreSystem() {
	FMOD::System* s = nullptr;
	fmodSystem->getCoreSystem(&s);
	return s;
}

void rd::AudioMan::traverseEvents(
	FMOD::Studio::Bank* bnk, 
	std::function<void(FMOD::Studio::EventDescription*)> proc){
	eastl::vector<std::string> eventList;
	int cnt = 0;
	bnk->getEventCount(&cnt);
	eastl::vector<Studio::EventDescription*> res;
	res.resize(cnt);
	bnk->getEventList(res.data(), cnt, &cnt);
	int mpath = 0;
	eventList.reserve(cnt);
	for (int i = 0; i < cnt; ++i) 
		proc( res[i] );
}

Str64 rd::AudioMan::GUIDtoStr(const FMOD_GUID& guid) {
	Str64 label;
	label.setf("{%08lX-%04hX-%04hX-%02hhX%02hhX-%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX}",
			guid.Data1, guid.Data2, guid.Data3,
			guid.Data4[0], guid.Data4[1], guid.Data4[2], guid.Data4[3],
			guid.Data4[4], guid.Data4[5], guid.Data4[6], guid.Data4[7]);
	return label;
}

void rd::AudioMan::GUIDtoStr(const FMOD_GUID&guid, Str&label) {
	label.setf("{%08lX-%04hX-%04hX-%02hhX%02hhX-%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX}",
				guid.Data1, guid.Data2, guid.Data3,
				guid.Data4[0], guid.Data4[1], guid.Data4[2], guid.Data4[3],
				guid.Data4[4], guid.Data4[5], guid.Data4[6], guid.Data4[7]);
}

bool rd::EventRef::isValid() const {//could scan banks?
	return !isEmpty();
}

bool rd::EventRef::isEmpty() const {
	return path.empty() && isGUIDEmpty();
}

bool rd::EventRef::isGUIDEmpty() const {
	bool headZero = (guid.Data1 != 0)
		|| (guid.Data2 != 0)
		|| (guid.Data3 != 0);
	if (headZero) return false;
	for (auto& v : guid.Data4)
		if (v != 0)
			return false;
	return true;
}

FMOD::Studio::EventDescription* rd::EventRef::getDesc() {
	return rd::AudioMan::get().getEvent(*this);
}

void rd::EventRef::clear() {
	path.clear();
	guid = {};
	bank.clear();
}

void rd::EventRef::stop() const{
	if (inst) {
		inst->stop(FMOD_STUDIO_STOP_ALLOWFADEOUT);
		inst->release();
		inst = 0;
	}
}

void rd::EventRef::playOnce() const {
	rd::AudioMan::get().playOnce(*this);
}

void rd::EventRef::play() const {
	rd::AudioMan::get().play(*this);
}

void rd::EventRef::im(){
	using namespace ImGui;
	Value("bank", bank);
	Value("path", path);
	
	Str l;
	rd::AudioMan::GUIDtoStr(guid, l);
	Value("guid", l);

	auto ref = ImGui::AudioPickerRef(true);
	if (ref)
		*this = *ref;
}

void rd::EventRef::serialize(Pasta::JReflect& jr){
	jr.visit(path, "path");
	jr.visit(bank, "bnk");
	Str lguid;
	if (jr.isWriteMode()) 
		rd::AudioMan::GUIDtoStr(guid,lguid);
	jr.visit(lguid, "guid");
	if (jr.isReadMode()) 
		FMOD::Studio::parseID( lguid.c_str(), &guid );
}

void rd::AudioMan::stopGraciously(const rd::EventRef& ref) {
	if (hasEvent(ref.path))
		stopGraciously(ref.path.c_str());
	else
		stopGraciously(ref.guid);
}

void rd::AudioMan::stopBrutally(const rd::EventRef& ref) {
	if (hasEvent(ref.path))
		stopBrutally(ref.path.c_str());
	else
		stopBrutally(ref.guid);
}