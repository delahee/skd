#include "stdafx.h"
#include <functional>
#include <string>
#include <vector>

#include "4-ecs/JsonReflect.h"
#include "1-files/PersistentMgr.h"

#ifdef _WIN32
#include "1-files/WinPersistentMgr.h"
#endif

#include "rd/JSerialize.hpp"
#include "rd/Anon.hpp"
#include "rd/String.hpp"
#include "rplatform/CrossPlatform.hpp"
#include "rs/File.hpp"

#include "Save.hpp"

using namespace std;
using namespace std::literals;


Profile*	Profile::me = 0;
Progress*	Progress::me = 0;
Save*		Save::me;
bool		Save::imOpened = false;

struct StubVec {

};

template<>
void Pasta::JReflect::visit(StubVec&, const char* name) {
	Save& sv = Save::get();
	visitObjectBegin("root");
	visit(sv.profiles, "profile_table");
	visitObjectEnd("root",false);
}


std::string					Save::folder = "hbc/dt";
#ifdef PASTA_FINAL
std::string					Save::filePrefix = "app_f_";
#else
std::string					Save::filePrefix = "app_d_";
#endif
std::string					Save::fileExt = ".sav";

std::string					Save::appDataDir = "";
std::string					Save::localDataDir = "";

std::function<void(Profile*)>	Save::createInitialProfile;
std::function<void(LocalPref*)> Save::createInitialPrefs;

Save::Save() {
	appDataDir =	rplatform::getAppData();
	localDataDir=	rplatform::getLocalAppData();

	if (!createInitialProfile) {
		createInitialProfile = [](Profile* prof) {
			//do nothing;
		};
	}

	if (!createInitialPrefs){
		createInitialPrefs = [](LocalPref* prof) {
			//do nothing;
		};
	}
}

Save& Save::get(){
	return *((me) ? me : (me = new Save()));
}

Profile::Profile() {
	progress = new Progress();
}

void Progress::serialize(Pasta::JReflect& jr, const char* _name){
	if (_name) jr.visitObjectBegin(_name);
	data.serialize(&jr);
	if (_name) jr.visitObjectEnd(_name);
}

enum WorldCompletion : int{
	NONE,
	KNOWN,
	TRIED,
	COMPLETED,

	USER_STATE = 16,
};

void Progress::reset(){
	data.dispose();
	dirty = true;
}

bool Progress::isJustCompleted(const char* label) {
	return data.getStr("last_level_completed") == label;
}

bool Progress::isCompleted(const char* label){
	return data.getInt( (std::string("complete_") + label).c_str()) == WorldCompletion::COMPLETED;
}

void Progress::completeLevel(const char* label) {
	data.set("last_level_completed",label);
	data.set( (Str256("complete_")+label).c_str(), WorldCompletion::COMPLETED);
}

bool Profile::im() {
	using namespace ImGui;
	LabelText("name", name.c_str() );
	Value("dirty",dirty);

	if (TreeNode("Config")) {
		config.im();
		TreePop();
	}
	if (TreeNode("Progress")) {
		progress->im();
		TreePop();
	}
	return false;
}

void Progress::im() {
	using namespace ImGui;
	data.im();
}

void Save::imWindow() {
	using namespace ImGui;

	if (Begin("Saves", &imOpened)) {
		im();
		End();
	}
}

void Save::im() {
	using namespace ImGui;
	if (lastResult.size())
		Value("Last result", lastResult);

	if (cachedPrefs) 
		cachedPrefs->im();
	else
		Text("No loaded local prefs");
	if (CollapsingHeader("Profile List")) {
		Indent();
		for (auto& p : profiles) {
			if (TreeNode(p.c_str())) {
				if (openedProfile&&openedProfile->name == p) {
					openedProfile->im();
					if (Button("Unload")) {
						closeProfile();
					}
				}
				else {
					if (Button("Load")) {
						closeProfile();
						openProfile(p);
					}
				}
				TreePop();
			}
		}
		Unindent();
	}

	if( openedProfile)
	if (CollapsingHeader("Active profile")) {
		openedProfile->im();
	}

	if (CollapsingHeader("Actions")) {
		if (Button("Save prefs")) {
			auto p = saveLocalPrefs();
			p->release();
			p->then([this](auto, auto) {
				lastResult = "Save Prefs success";
				return 0;
			}, [this](auto, auto) {
				lastResult = "Save Prefs FAILED";
				return 0;
			});
		}

		if (openedProfile) {
			if (Button("Save profile")) {
				auto p = saveOpenedProfile();
				p->release();
				p->then([this](auto, auto) {
					lastResult = "Save Prof success";
					return 0;
					}, [this](auto, std::any _err) {
						const char* c_err = _err.type() != typeid(std::string) ? std::any_cast<const char*>(_err) : 0;
						std::string std_err;
						if (!c_err)
							std_err = std::any_cast<string>(_err);
						lastResult = string("Save Prof FAILED ") + ((!c_err) ? std_err : c_err);
						return 0;
					});
			}
		}
		else {
			ImGui::Text("No opened profile");
		}

		if (Button("Close profile")) {
			auto p = closeProfile();
			p->release();
			p->then([this](auto, auto) {
				return 0;
			}, [this](auto, std::any _err) {
				return 0;
			});
		}
	}
}

template<> inline void Pasta::JReflect::visit(LocalPref& m,const char * name) {
	m.serialize(*this, name);
}

LocalPref* Save::openLocalPrefsSync(){
	if(cachedPrefs) return cachedPrefs;

	cachedPrefs = new LocalPref();
	std::string path = localDataDir + "/hbc/dt/localprefs.json";
	std::string content;
	bool fileRead = rs::File::read(path,content);
	if (!fileRead) {
		cout << "cannot open pp : creating them"<<endl;
		createInitialPrefs(cachedPrefs);
		saveLocalPrefs();
		return cachedPrefs;
	}
	cout << "Local prefs opened" << endl;
	jDeserializeFromString(*cachedPrefs, content);
	return cachedPrefs;
}

Promise* Save::openLocalPrefsAsync() {
	return Promise::getSuccess(openLocalPrefsSync());
}

static const char* profileListFile = "profiles.txt";

Promise* Save::saveLocalPrefs() {
	if (!cachedPrefs) return Promise::getFailure("no player prefs cached to save");

	auto mgr = Pasta::FileMgr::getSingleton();
	std::string dir = localDataDir + "/hbc/dt/";
	bool dok0 = rs::File::createDir(localDataDir+"/hbc");
	bool dok1 = rs::File::createDir(localDataDir+"/hbc/dt");
	std::string path = dir + "localprefs.json";
	std::string content = jSerializeToString(*cachedPrefs);
	bool fok = rs::File::write(path, content);
	return fok ? Promise::getSuccess(string("saved")):Promise::getFailure(string("error"));
}

bool Save::saveProfileList() {
	Pasta::PersistentMgr* mgr = Pasta::PersistentMgr::getSingleton();
	StubVec sv;
	string data = jSerializeToString(sv);
	mgr->openSave("dt", Pasta::SaveOpenMode::Write);
	auto resWrite = mgr->writeData("fat.data", data.data(),data.size(), Pasta::SaveTaskFlags::DisableHeader);
	mgr->closeSave();
	return resWrite == Pasta::SaveError::NoError;
}

std::vector<std::string> Save::listProfiles(){
	if (profiles.size()==0) {
		Pasta::PersistentMgr* mgr = Pasta::PersistentMgr::getSingleton();
		void* data=0;
		int sz = 0;
		mgr->openSave("dt",Pasta::SaveOpenMode::Read);
		auto resWrite = mgr->readData("fat.data", &data, &sz, Pasta::SaveTaskFlags::DisableHeader);
		if (sz) {
			StubVec sv;
			string str = string((char*)data, sz)+'\0';
			jDeserializeFromString(sv,str);
			mgr->releaseData(data);
			mgr->closeSave();
		}
	}
	return profiles;
}

Promise* Save::createProfile(const std::string& name) {
	if(openedProfile)
		return Promise::getFailure("close previous first");
	if (rs::Std::exists(listProfiles(), name))
		return Promise::getFailure("already exists");

	openedProfile = new Profile();
	openedProfile->name = name;
	createInitialProfile(openedProfile);
	profiles.push_back(name);
	saveProfileList();
	cout << "createProfile done" << endl;
	return saveOpenedProfile();
}

Promise* Save::openProfile(const string & name){
	auto profs = listProfiles();
	if (!rs::Std::exists(profs, name))
		return Promise::getFailure("profile does not exist");

	if (!openedProfile) {
		Profile* prof = new Profile();
		prof->name = name;
		openedProfile = prof;
		std::string content;
		bool ok = rs::File::readSaveFile(openedProfile->name,content);
		if (!ok) {
			openedProfile = nullptr;
			delete prof;
			Progress::me = nullptr;
			return Promise::getFailure("cannot read");
		}
		jDeserializeFromString(*prof, content);
		Progress::me = openedProfile->progress;
		return Promise::getSuccess(prof);
	}
	else if (openedProfile->name == name) {
		auto p = new Promise();
		p->resolve(openedProfile);//already opened
		return p;
	}
	else{
		closeProfile();
		return openProfile(name);
	}
}

Promise* Save::saveOpenedProfile(){
	if (!openedProfile)  return Promise::getFailure(string("no profile opened"));
	auto prof = openedProfile;
	string doc = jSerializeToString(*prof);
	bool ok = rs::File::writeSaveFile(openedProfile->name,doc);
	if (!ok) return Promise::getFailure(string("cannot write file"));
	return Promise::getSuccess(prof);
}

Promise* Save::saveOpenedProfileIfNeeded(){
	if(!openedProfile) Promise::getFailure(string("no profile opened"));
	if (openedProfile && openedProfile->dirty)
		return saveOpenedProfile();
	else
		return Promise::getSuccess(openedProfile);
}

Promise* Save::closeProfile(){
	if (openedProfile) {
		Progress::me = nullptr;
		delete openedProfile;
		openedProfile = 0;
	}
	return Promise::getSuccess(string("success"));
}

Profile* Save::getOpenedProfile()
{
	return openedProfile;
}

void Profile::serialize(Pasta::JReflect& jr, const char* _name) {
	if (_name) jr.visitObjectBegin(_name);
	jr.visit(name, "name");
	config.serialize(&jr,"config");
	progress->serialize(jr,"progress");
	if (_name) jr.visitObjectEnd(_name);
}

void LocalPref::im() {
	using namespace ImGui;
	if (TreeNode("Local Prefs")) {
		data.im();
		TreePop();
	}
}

void LocalPref::serialize(Pasta::JReflect& jr, const char* _name){
	data.serialize(&jr, _name);
}
