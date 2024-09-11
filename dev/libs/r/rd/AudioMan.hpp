#pragma once 

#include <array>
#include "fmod_studio.hpp"
#include "fmod.hpp"
#include "r/Types.hpp"
#include "rd/Sig.hpp"

namespace Pasta			{	struct JReflect; }
namespace FMOD			{
	namespace Studio {
		class Bank;	class EventDescription; class EventInstance;
	}
}

namespace ra {
	typedef FMOD::Studio::EventInstance		EventInstance;
	typedef FMOD::Studio::EventDescription	EventDesc;
}

namespace rd {
	struct AudioManConf {
		Vector3 FWD = Vector3(0, 1.0f, 0);
		Vector3 UP = Vector3(0, 0, 1.0f);

		AudioManConf() {}
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

	enum class EventRefFlags : r::u32 {
		None = 0,
	};

	struct EventRef {
		EventRefFlags	flags;
		Str				bank;
		Str				path;
		FMOD_GUID		guid = {};
		mutable FMOD::Studio::EventInstance* 
						inst = {};

						EventRef() {};
						EventRef(const char * path, const char * guid);
						EventRef(const char * path, const FMOD_GUID& guid);
						
		void			serialize(Pasta::JReflect& jr);
		bool			isGUIDEmpty() const;
		bool			isEmpty() const;
		bool			isValid() const;
		bool			hasInstance() const;

		FMOD::Studio::EventDescription*
						getDesc();
		void			clear();
		void			im();
		void			stop() const;
		void			play() const;
		void			playOnce() const;
	};
	

	class AudioMan;

	class FilePlayer {
	public:
		AudioMan*										am = nullptr;
		std::unordered_map<Str, FMOD::Sound*>			files;
		std::unordered_map<Str, FMOD::Channel*>			chns;

		bool											hasOnDisk(const char* name);
		bool											hasInCache(const char* name);
		void											load(const char* name, bool loop = false);
		void											play(const char* name, float vol = 1.0f, bool loop = false);
		void											stop(const char* name);
		void											im();
	};

	
	class AudioMan {
	public:
		enum LOG_MASK{
			LOG_PLAY_FILE = 1<<0,
			LOG_PLAY_EVENT = 1<<1,
		};
		static AudioMan*										me;

		std::array<AudioManConf, (int)AudioManConfStyle::AMCS_COUNT> 
																confs;
		AudioManConfStyle										confStyle;
		AudioManConf											conf;

		//see LOG_MASK
		r::u32													logs = 0;

		bool													initialized = false;

		rd::Sig													onSelection;

		bool													debug = false;
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
		rd::MsgChanB											onMute;

																AudioMan& operator=(const AudioMan&) = delete;
																AudioMan(const AudioMan&) = delete;

		static AudioMan&										get();

		void													init();
		void													destroy();//might be needed if you want to shut fmod?
		void													unloadAll();

		void													setListenerPos(const Vector3& v);
		void													syncListener();

		FMOD::Studio::EventInstance*							playOnceSilentErrors(const char* event);

		double													getDuration(FMOD::Studio::EventDescription&ed);
		double													getDuration(FMOD::Studio::EventInstance&ei);

		//same as play once but not released
		FMOD::Studio::EventInstance*							play(const char * ev);
		FMOD::Studio::EventInstance*							play(const FMOD_GUID&guid);
		FMOD::Studio::EventInstance*							play(const rd::EventRef&ref);
		FMOD::Studio::EventInstance*							play(const std::string& event);

		FMOD::Studio::EventInstance*							playOnce(const rd::EventRef&ref);
		FMOD::Studio::EventInstance*							playOnce(const char* event);
		FMOD::Studio::EventInstance*							playOnce(const FMOD_GUID& guid);
		FMOD::Studio::EventInstance*							playOnce(const std::string& event);
		FMOD::Studio::EventInstance*							playOnce(const std::string& event, const std::unordered_map<std::string,float> &params);
		FMOD::Studio::EventInstance*							playOnce(const rd::EventRef& ref, const std::unordered_map<std::string,float> &params);

		bool													hasFile(const char* fname);
		bool													playFile(const char* fname ,float vol = 1.0f, bool loop = false);
		bool													playFile(const std::string& fname, float vol = 1.0f, bool loop = false ) { return playFile(fname.c_str(), vol, loop); };

		bool													preloadFile(const char* fname );
		bool													stopFile(const char* fname );

		bool													isPlaying(const char* evName);
		bool													isPlayingEvent(const char* evName);
		bool													isPlayingFile(const char* fname);
		bool													isPlaying(ra::EventInstance*ei);

		void													stopAll(const std::string& event);
		void													stopAll(const char* ev);
		void													stopAll(const rd::EventRef& ref);

		void													stopGraciously(const FMOD_GUID& ev);
		void													stopGraciously(const char* ev);
		void													stopGraciously(const rd::EventRef& ref);

		void													stopBrutally(const char* ev);
		void													stopBrutally(const FMOD_GUID& ev);
		
		void													stopBrutally(const rd::EventRef& ref);

		void													setConfStyle(AudioManConfStyle acs);

		bool													hasBank(const char* bnk) { return getBank(bnk); };
		bool													hasBank(rd::EventRef& ev);

		bool													hasInternalLoop(FMOD::Studio::EventDescription* ed);
		bool													isOneShot(FMOD::Studio::EventDescription* ed);

		bool													hasEvent(rd::EventRef& ref);
		bool													hasEvent(const char* ev);
		bool													hasEvent(const FMOD_GUID& ref);
		bool													hasEvent(const Str& ev) { return 0 != hasEvent(ev.c_str()); };

		FMOD::Studio::EventDescription*							getEvent(const rd::EventRef& ref);
		FMOD::Studio::EventDescription*							getEvent(const FMOD_GUID& ref);
		FMOD::Studio::EventDescription*							getEvent(const char* ev);
		FMOD::Studio::EventDescription*							getEvent(const std::string& ev) { return getEvent(ev.c_str()); };
		FMOD::Studio::EventDescription*							getEvent(const Str& ev) { return getEvent(ev.c_str()); };

		void													getEvent(const char* ev, FMOD::Studio::EventDescription*& desc);
		void													getEvent(const std::string& ev, FMOD::Studio::EventDescription*& desc);

		eastl::vector<Str>										getEventList(const char* bnk);

		//release it on end please, be careful with direct calls
		FMOD::Studio::EventInstance*							getEventInstance(const std::string& ev);
		FMOD::Studio::EventInstance*							getEventInstance(const char* ev);
		void													getEventInstance(const char* ev, FMOD::Studio::EventInstance*& evi);
		FMOD::Studio::EventInstance*							getEventInstance(FMOD::Studio::EventDescription*& desc);
		void													getEventInstance(FMOD::Studio::EventDescription*& desc, FMOD::Studio::EventInstance*& evi);

		FMOD::Studio::EventInstance*							getActiveEventInstance(const char* ev);

		//should start with bus:/ otherwise returns the main bus
		FMOD::Studio::Bus*										getBus(const char * path = 0);
		FMOD::Studio::Bus*										getMainBus();
		//should start with vca:/ otherwise returns the main vca is exists
		FMOD::Studio::VCA*										getVCA(const char* path = 0);
		std::string												getVCAName(FMOD::Studio::VCA* vca);
		FMOD::Studio::VCA*										getMainVCA();
		bool													muteVCA(const char * path );
		eastl::vector<FMOD::Studio::VCA*>						getAllVCA();

		FMOD::Studio::Bank*										getMasterBank();
		FMOD::Studio::Bank*										getBank(const char* bnk);
		void													loadBankIfNot(const char* bnk);

		void													im();
		void													imEventInst(FMOD::Studio::EventInstance*inst);
		void													imEventDesc(FMOD::Studio::EventDescription*desc);
		void													serialize(Pasta::JReflect& refl);

		void													prefetch(const std::string& bnkName);
		void													prefetch(const char * bnk, bool addStrBnk = false);
		void													fullFetch(const char* _bnk);

		eastl::vector<Str>										getBankList();

		void													unload(const char* bnk);

		void													ev_setPos(FMOD::Studio::EventInstance* ev, const Vector3& pos);

		void													getParamTypeName(FMOD_STUDIO_PARAMETER_TYPE t,std::string & ret);
		void													getParamFlagDesc(FMOD_STUDIO_PARAMETER_FLAGS t, std::string& ret);

		bool													hasParameter(FMOD::Studio::EventInstance & ev, const char* name);
		bool													hasParameter(const char * prm);

		FMOD_RESULT												setParameter(const char* prm, float value);
		FMOD_RESULT												setParameter(const char* prm, const char* value);

		bool													isMute();
		void													setMute(bool mute);
		FMOD::System*											getCoreSystem();

		void													traverseEvents(FMOD::Studio::Bank* bnk, std::function<void(FMOD::Studio::EventDescription*)> proc);
		static void												GUIDtoStr(const FMOD_GUID& guid, Str& label);
		static Str64											GUIDtoStr(const FMOD_GUID& guid);
	protected:
		AudioMan();

		FilePlayer												fp;
		float													latestUserMasterBusVolume = 1.0f;
		float													latestUserMasterVCAVolume = 1.0f;
	};
}

bool FMOD_CHECK(FMOD_RESULT res, const char * addMsg = 0);

inline FMOD_VECTOR v3ToFMOD(r::Vector3 const& v) {
	FMOD_VECTOR fmv = {};
	fmv.x = v.x;
	fmv.y = v.y;
	fmv.z = v.z;
	return fmv;
};

namespace ra {
	typedef rd::EventRef EventRef;
}

eastl::vector<FMOD_STUDIO_USER_PROPERTY> getUserProperties(FMOD::Studio::EventDescription* desc);