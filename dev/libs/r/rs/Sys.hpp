#pragma once

#include <cstdlib>
#include <vector>
#include <functional>
#include <algorithm>
#include <utility>

#include "rd/Agent.hpp"
#include "IEventListener.hpp"
#include "InputEvent.hpp"

namespace r2 { class Node; }
namespace rd { class Tweener; }

namespace rs{
	class Sys {
	public:
		static double			FPS;
		static void				printTime(const char * s = nullptr);
		static inline int		random(int n) {
			return n * (int)rand() / RAND_MAX;
		};
		static inline double	randf() {
			return 1.0 * rand() / RAND_MAX;
		};

		static std::vector< IEventListener * >	
										eventListeners;
	
		static void						addEventListener(IEventListener * e);
		static void						removeEventListener(IEventListener * e);
		static void						event(InputEvent & ev);

		static double					lastMouseActivity;
		static double					lastKeyActivity;

		static bool						isMousePressed;
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
		static int						LOG_LEVEL;

	private:
		static void sysAsserts();
	};
}

namespace rs {
	class StackTrace {
	public:
		static Str	versionLabel;
		static void init();
	};

	void			abortHandler(int signum);

	void			sysConsolePrint(const char * msg);
			
			void	traceWarning(const char * msg);
	inline	void	traceWarning(const std::string& msg)	{ traceWarning(msg.c_str()); };
	inline	void	traceWarning(const Str& msg)			{ traceWarning(msg.c_str()); };
			
			void	traceError(const char* msg);
	inline	void	traceError(const std::string& msg)	{ traceError(msg.c_str()); };
	inline	void	traceError(const Str& msg)			{ traceError(msg.c_str()); };
	
			void	traceGreenlight(const char* msg);
	inline	void	traceGreenlight(const std::string& msg) { traceGreenlight(msg.c_str()); };
	inline	void	traceGreenlight(const Str& msg)			{ traceGreenlight(msg.c_str()); };

			void	trace(const char * msg, int level );
			void	trace(const char * msg);
	
#ifdef PASTA_DEBUG
	#define dtrace( msg ) { trace(msg); }
#else
	#define dtrace( ... ) {}
#endif
	
			void	trace(const std::string & msg);
	inline	void	trace(const Str& msg) { trace(msg.c_str()); };
	//void			tracef(const char* format, ...);

	void			traceObject(const char* prefix, void* obj);
	inline void		traceObject(const std::string& prefix, void* obj) { traceObject(prefix.c_str(), obj); };
	void            traceAgent(const char* prefix, rd::Agent* ag);

	void			traceNode(const char* prefix, r2::Node* obj);

	inline	void	nop() {};
}