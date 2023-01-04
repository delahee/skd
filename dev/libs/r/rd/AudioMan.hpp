#pragma once 

#include <array>
#include "fmod_studio.hpp"
#include "fmod.hpp"
#include "r/Types.hpp"
#include "rd/Sig.hpp"

namespace Pasta {
	struct JReflect;
}

namespace rd {
	struct AudioManConf {
		Vector3 FWD = Vector3(0, 1.0f, 0);
		Vector3 UP = Vector3(0, 0, 1.0f);

		AudioManConf() {

		}

		AudioManConf(Vector3 fwd, Vector3 up) {
			FWD = fwd;
			UP = up;
		}
	};

	enum class AudioManConfStyle : u32 {
		AMCS_PIXELS,
		AMCS_ISO,
		AMCS_COUNT,
	};

	class AudioMan;

	class FilePlayer {
	public:
		AudioMan*								am = nullptr;
		std::map<std::string, FMOD::Sound*>		files;

		void									load(const char* name);
		void									play(const char* name, float vol = 1.0f);
	};

	class AudioMan {
	public:
		static AudioMan*										me;


		std::array<AudioManConf, (int)AudioManConfStyle::AMCS_COUNT> confs;
		AudioManConfStyle										confStyle;
		AudioManConf											conf;

		bool													initialized = false;

		rd::Sig													onSelection;

		bool													allow3dListener = false;

		//if you want to skip audio event processing to minimize audio pressure...or skip some audio events when simulating situations
		bool													enableAudioProcessing = true;

		Vector3													listenerPos;
		Vector3													listenerFwd;
		Vector3													listenerUp;
		Vector3													listenerVel;

		FMOD::Studio::System*									fmodSystem = nullptr;
		FMOD::Studio::Bank*										masterBank = nullptr;
		std::unordered_map<std::string, FMOD::Studio::Bank*>	banks;

		std::string												intentEvent;
		std::string												intentBank;

		eastl::vector<Str>										eventLog;

		AudioMan& operator=(const AudioMan&) = delete;
		AudioMan(const AudioMan&) = delete;

		static AudioMan& get();

		void													init();
		void													destroy();//might be needed if you want to shut fmod?
		void													unloadAll();

		void													setListenerPos(const Vector3& v);
		void													syncListener();

		FMOD::Studio::EventInstance*							playOnce(const char* event);
		FMOD::Studio::EventInstance*							playOnceSilentErrors(const char* event);

		//same as play once but not released
		FMOD::Studio::EventInstance*							play(const std::string& event);
		FMOD::Studio::EventInstance*							playOnce(const std::string& event);
		FMOD::Studio::EventInstance*							playOnce(const std::string& event, const std::unordered_map<std::string,float> params);

		bool													playFile(const char* fname ,float vol = 1.0f);

		void													stopAll(const std::string& event);
		void													stopAll(const char* ev);

		void													setConfStyle(AudioManConfStyle acs);

		FMOD::Studio::EventDescription*							getEvent(const char* ev);
		void													getEvent(const char* ev, FMOD::Studio::EventDescription*& desc);
		void													getEvent(const std::string& ev, FMOD::Studio::EventDescription*& desc);

		eastl::vector<std::string>								getEventList(const char* bnk);
		//release it on end
		FMOD::Studio::EventInstance*							getEventInstance(const std::string& ev);
		FMOD::Studio::EventInstance*							getEventInstance(const char* ev);
		void													getEventInstance(const char* ev, FMOD::Studio::EventInstance*& evi);
		FMOD::Studio::EventInstance*							getEventInstance(FMOD::Studio::EventDescription*& desc);
		void													getEventInstance(FMOD::Studio::EventDescription*& desc, FMOD::Studio::EventInstance*& evi);

		//should start with bus:/ otherwise returns the main bus
		FMOD::Studio::Bus*										getBus(const char * path = 0);
		FMOD::Studio::Bus*										getMainBus();
		//should start with vca:/ otherwise returns the main vca is exists
		FMOD::Studio::VCA*										getVCA(const char* path = 0);
		FMOD::Studio::VCA*										getMainVCA();

		FMOD::Studio::Bank*										getBank(const char* bnk);

		void													im();
		void													serialize(Pasta::JReflect& refl);

		void													prefetch(const std::string & bnk);
		std::vector<std::string>								getBankList();

		void													unload(const char* bnk);

		void													ev_setPos(FMOD::Studio::EventInstance* ev, const Vector3& pos);

		void													getParamTypeName(FMOD_STUDIO_PARAMETER_TYPE t,std::string & ret);
		void													getParamFlagDesc(FMOD_STUDIO_PARAMETER_FLAGS t, std::string& ret);

		bool													isMute();
		void													setMute(bool onOff);
		FMOD::System*											getCoreSystem();
	protected:
		AudioMan();

		FilePlayer												fp;
		float													latestUserMasterBusVolume = 1.0f;
		float													latestUserMasterVCAVolume = 1.0f;
	};
}

bool FMOD_CHECK(FMOD_RESULT res);

inline FMOD_VECTOR v3ToFMOD(r::Vector3 const& v) {
	FMOD_VECTOR fmv = {};
	fmv.x = v.x;
	fmv.y = v.y;
	fmv.z = v.z;
	return fmv;
};