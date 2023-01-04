#pragma once
#include <string>
#include "rd/Promise.hpp"
#include <vector>
#include "rd/Vars.hpp"
#include "rd/JSerialize.hpp"

using namespace rd;

class Progress;

class Profile {
public:

	Profile();
	static Profile* me;
	bool			dirty = false;

	std::string		name;
	rd::Vars		config;//store various xplat config stuff
	Progress*		progress=0;//store progress
	void			im();
	void			serialize(Pasta::JReflect&jr,const char*_name = 0);

};

class Progress {
public:
	static Progress* me;

	bool	 dirty = false;
	rd::Vars data;

	void		im();
	void		serialize(Pasta::JReflect& jr, const char* _name = 0);
	void		reset();
	bool		isJustCompleted(const char* label);
	bool		isCompleted(const char * label);
	void		completeLevel(const char* label);

};

class LocalPref {
public:
	bool	 dirty = false;
	rd::Vars data;

	void	 im();
	void	 serialize(Pasta::JReflect& jr, const char* _name = 0);
};

class Save {//don't make it an agent yet
public:
	static Save* me;
	static Save& get();

	void							imWindow();

	void							im();

	static string					folder;
	static string					filePrefix;
	static string					fileExt;

	static string					appDataDir;
	static string					localDataDir;

	static bool						imOpened;

	inline	LocalPref*				safeGetLocalPrefs() { openLocalPrefsSync();  return cachedPrefs; };
	LocalPref*						getLocalPrefs() { return cachedPrefs; };
	LocalPref*						openLocalPrefsSync();
	Promise*						openLocalPrefsAsync();
	Promise*						saveLocalPrefs();

	int								getProfileCount() { return listProfiles().size(); }
	std::vector<std::string>		listProfiles();

	bool							saveProfileList();
	Promise*						createProfile(const std::string& name);

	//screams back if not created first
	Promise*						openProfile(const std::string & name);
	Promise*						saveOpenedProfile();
	Promise*						saveOpenedProfileIfNeeded();
	Promise*						closeProfile();

	Profile*						getOpenedProfile();

	static std::function<void(Profile*)>	createInitialProfile;
	static std::function<void(LocalPref*)>	createInitialPrefs;

	vector<string>					profiles;

private:
	string							lastResult;
	LocalPref*						cachedPrefs = 0;
	Profile*						openedProfile = 0;
	Save();
	Save(const Save& _s) = delete;
};