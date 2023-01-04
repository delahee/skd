#pragma once

#include <stdlib.h>
#include <vector>
#include <functional>
#include <algorithm>
#include <utility>

#include "rd/Agent.hpp"
#include "IEventListener.hpp"
#include "InputEvent.hpp"

namespace rd { class Tweener; }

namespace rs{
	class Sys {
	public:
		static double FPS;

		static void printTime(const char * s = nullptr);

		static inline int random(int n) {
			return n * (int) rand() / RAND_MAX;
		}

		static inline double randf() {
			return 1.0 * rand() / RAND_MAX;
		}

		static std::vector< IEventListener * > eventListeners;
	
		static void						addEventListener(IEventListener * e);
		static void						removeEventListener(IEventListener * e);

		static void						event(InputEvent & ev);

		static bool						isMousePressed	;
		static bool						hasInputKeyThisFrame;

		static int						mousePressedMask;

		static int						prevMouseX;
		static int						prevMouseY;

		static int						mouseX;
		static int						mouseY;
		

		static bool						isButtonJustPressed(Pasta::ControllerType ctrl, Pasta::Key key);
		static bool						isButtonJustReleased(Pasta::ControllerType ctrl, Pasta::Key key);
		static bool						mouseWasPreviouslyPressed();
		static void						updateInputs();

		static void						init();

		//keeps system in a "runnable" state
		//removing all processes
		static void						dispose();

		static std::vector<InputEvent>	keyEvents;
		static unsigned int				uintFromString(const char * val);

		static uint64_t					getUID();
		static void						reserveUID(uint64_t uid);

		static void						enterFrame();
		static void						exitFrame();

		static std::string				bootDirectory;

		static rd::AgentList			enterFrameProcesses;
		static rd::AgentList			exitFrameProcesses;
		static std::function<void(const char*)> traceOverride;

		static bool						filePickForOpen(std::string & result);
		static bool						filePickForOpen(const std::vector<std::pair<std::string, std::string>> & filters, std::string & result);
		static bool						filePickForSave(const std::vector<std::pair<std::string, std::string>> & filters, std::string & result);

		static bool						filePickForOpen(const std::pair<std::string, std::string> & filter, std::string& result) {
			std::vector<std::pair<std::string, std::string>>  all;
			all.push_back(filter);
			return filePickForOpen(all, result);
		};

		static r::u32					CRC32(const std::string& str);
		static r::u32					Adler32(const std::string& str);
		static rd::Tweener*				tw;
	private:
		static void sysAsserts();
	};
}

namespace rs {
	class StackTrace {
	public:
		static void init();
	};

	void sysConsolePrint(const char * msg);


	void trace(const char * msg);
	void trace(const std::string & msg);
	inline void trace(const Str& msg) { trace(msg.c_str()); };
}