#pragma once

namespace sol {
	class state;
};

#include "Agent.hpp"

namespace rd {
	class LuaScriptHost : public rd::Agent {
		typedef rd::Agent Super;

        bool						deleteAsked = false;
        eastl::vector<r2::Node*>	destroyList;
	public:
		static bool				globalStateInit;
		std::string				defaultFunc;

		sol::environment		localEnvironment;
		sol::unsafe_function	funcRun;
		bool					evalEachFrame = false;

		bool					init = false;
		bool					watchFile = true;
		Pasta::File *			file = nullptr;
		std::string				filePath;
		Pasta::u64				lastReadTime = 0;

		float					DELTA_CHECK = 0.1f;
		float					checkProgress = 0.0f;

		Sig						onBeforeEval;
		Sig						onDestruction;
		Sig						onAfterContextCreation;
		Sig						onAfterEval;
		Sig						onBuildContext;

		std::function<void(const std::string & err)>					
								log;
	public:
								LuaScriptHost(const std::string & _scriptPath, AgentList * _al, std::string _defaultFunc = "run");
		virtual					~LuaScriptHost();

		sol::state&				getEngine();
		virtual void			buildContext();
		virtual void			updateContext(sol::state& luaSol);

		static	void			injectR(sol::state & luaSol);
#ifndef HBC_NO_LUA_IMGUI
		static	void			injectImGui(sol::state & luaSol);
#endif
#ifndef HBC_NO_LUA_FMOD
		static	void			injectFmod(sol::state & luaSol);
#endif

		void					callFunc(const char * name);

		virtual void			update(double dt);
		virtual void			eval();
		virtual void			reeval();

		bool					im() override;
	};
}//end namespace rd