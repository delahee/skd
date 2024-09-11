#include "Sys.hpp"
#include "stdafx.h"
#include <cstring>
#include <codecvt>

#ifdef _WIN32
#include <windows.h>
#include <commdlg.h>
#include <wchar.h>
#include "1-input/WinInputMgr.h"
#include "1-graphics/HWNDWindowMgr.h"
#else //unix
#include <unistd.h>
#endif

#include "1-texts/TextMgr.h"
#include "1-input/InputMgr.h"
#include "2-mail/SendMailMgr.h"
#include "4-menus/imgui.h"

#include "IEventListener.hpp"
#include "InputEvent.hpp"
#include "Sys.hpp"
#include "rd/Tween.hpp"
#include "rd/Garbage.hpp"
#include "rplatform/CrossPlatform.hpp"
#include "rplatform/Device.hpp"
#include "rd/JSerialize.hpp"
#include "rd/Console.hpp"
#include "rs/InputEvent.hpp"

using namespace Pasta;
using namespace std;
using namespace rs;


double Sys::FPS = 60.0;

rd::AgentList rs::Sys::enterFrameProcesses;
rd::AgentList rs::Sys::exitFrameProcesses;

std::vector< rs::IEventListener * > rs::Sys::eventListeners;
std::string rs::Sys::bootDirectory;
bool rs::Sys::hasInputKeyThisFrame=false;
int rs::Sys::LOG_LEVEL = 0;

double rs::Sys::lastMouseActivity = -1.0;
double rs::Sys::lastKeyActivity = -1.0;

static Pasta::u64 initTime = Pasta::Time::getTimeMarker();

void rs::Sys::printTime(const char * s){
	if (s == nullptr) s = "";
	Pasta::u64 mark = Pasta::Time::getTimeMarker() - initTime;
	double tms = Pasta::Time::getTimeNS(mark) / 1000000000.0;
	printf("tick [%s] : %f s\n", s,tms);
}

void rs::Sys::addEventListener( rs::IEventListener * e){
	eventListeners.push_back(e);
}

void rs::Sys::removeEventListener( rs::IEventListener * e ){
	auto find = std::find(rs::Sys::eventListeners.begin(), rs::Sys::eventListeners.end(), e);
	if(find != rs::Sys::eventListeners.end())
		rs::Sys::eventListeners.erase(find);
}

void rs::Sys::event(InputEvent & ev) {
	for (auto it : eventListeners) {
		it->onEvent(ev);
	}
}

int		rs::Sys::mousePressedMask = 0;
bool	rs::Sys::isMousePressed = false;

int		rs::Sys::prevMouseX = -1;
int		rs::Sys::prevMouseY = -1;

int		rs::Sys::mouseX	= -1;
int		rs::Sys::mouseY	= -1;

bool rs::Sys::isButtonJustPressed(ControllerType ctrl, Key key) {
	InputMgr* mgr = InputMgr::getSingleton();
	return mgr->keyIsPressed(ctrl, key) && !mgr->keyWasPreviouslyPressed(ctrl, key);
}

bool rs::Sys::isButtonJustReleased(ControllerType ctrl, Key key) {
	InputMgr* mgr = InputMgr::getSingleton();
	return !mgr->keyIsPressed(ctrl, key) && mgr->keyWasPreviouslyPressed(ctrl, key);
}

bool rs::Sys::mouseWasPreviouslyPressed() {
	InputMgr * mgr = InputMgr::getSingleton();
	return mgr->keyWasPreviouslyPressed(CT_MOUSE, MOUSE_LEFT) || mgr->keyWasPreviouslyPressed(CT_MOUSE, MOUSE_RIGHT) || mgr->keyWasPreviouslyPressed(CT_MOUSE, MOUSE_MIDDLE);
}

static std::unordered_map<int, bool> downs;
void rs::Sys::updateInputs(){
	double now = rs::Timer::now;
	int prevX = prevMouseX = mouseX;
	int prevY = prevMouseY = mouseY;
	isMousePressed = false;
	mouseX = -1;
	mouseY = -1;
	if (lastKeyActivity < 0) lastKeyActivity = now;
	if (lastMouseActivity< 0) lastMouseActivity = now;

	InputMgr * mgr = InputMgr::getSingleton();
#ifndef PASTA_NX
	{
		///mouse move
		u16 x = UINT16_MAX;
		u16 y = UINT16_MAX;
		mgr->getStylusXY(&x, &y);

		mousePressedMask = 0;
		if (mgr->keyIsPressed(CT_MOUSE, MOUSE_LEFT))	mousePressedMask |= BUTTON_LEFT;
		if (mgr->keyIsPressed(CT_MOUSE, MOUSE_RIGHT))	mousePressedMask |= BUTTON_RIGHT;
		if (mgr->keyIsPressed(CT_MOUSE, MOUSE_MIDDLE))	mousePressedMask |= BUTTON_MIDDLE;
		isMousePressed = mousePressedMask != 0;

		mouseX = x;
		mouseY = y;

		int newlyPressed = 0;
		int newlyReleased = 0;

		if (x == UINT16_MAX || y == UINT16_MAX) {
			//no events
		} else {
			if (isButtonJustPressed(CT_MOUSE, MOUSE_LEFT))		newlyPressed |= BUTTON_LEFT;
			if (isButtonJustPressed(CT_MOUSE, MOUSE_RIGHT))		newlyPressed |= BUTTON_RIGHT;
			if (isButtonJustPressed(CT_MOUSE, MOUSE_MIDDLE))	newlyPressed |= BUTTON_MIDDLE;
			
			if (isButtonJustReleased(CT_MOUSE, MOUSE_LEFT))		newlyReleased |= BUTTON_LEFT;
			if (isButtonJustReleased(CT_MOUSE, MOUSE_RIGHT))	newlyReleased |= BUTTON_RIGHT;
			if (isButtonJustReleased(CT_MOUSE, MOUSE_MIDDLE))	newlyReleased |= BUTTON_MIDDLE;

			if (newlyPressed) {
				InputEvent ev(InputEventKind::EIK_Push, x, y);
				ev.origin = InputOrigin::Mouse;
				ev.button = newlyPressed;
				event(ev);
			}
			if (newlyReleased) {
				InputEvent ev(InputEventKind::EIK_Release, x, y);
				ev.origin = InputOrigin::Mouse;
				ev.button = newlyReleased;
				event(ev);
			}
			if (prevX != x || prevY != y) {
				InputEvent ev(InputEventKind::EIK_Over, x, y);
				ev.origin = InputOrigin::Mouse;
				ev.button = mousePressedMask;
				event(ev);
			}
		}

		//wheel
		float val = mgr->getKeyValue(CT_MOUSE, MOUSE_WHEEL);
		if (val != 0.f) {
			InputEvent ev(InputEventKind::EIK_Wheel, x,y);
			ev.origin = InputOrigin::Mouse;
			ev.wheelDelta = val;
			event(ev);
		}

		if ((prevMouseX != mouseX) || (prevMouseY != mouseY)) 
			lastMouseActivity = now;

		if(newlyPressed || newlyReleased)
			lastMouseActivity = now;
	}

#endif

	auto io = ImGui::GetIO();
	for (int i = 0; i < io.InputQueueCharacters.size(); i++) {
		wchar_t c = io.InputQueueCharacters[i];
		if( c != 0 ) {
			InputEvent e(InputEventKind::EIK_Character);
			e.origin = InputOrigin::Keyboard;
			e.relX = -1;
			e.relY = -1;
			e.charCode = c;
			rs::Sys::keyEvents.push_back(e);
		}
	}
	
	if (rs::Sys::keyEvents.size()) {
		for (auto e : rs::Sys::keyEvents)
			event(e);
		rs::Sys::keyEvents.clear();
		hasInputKeyThisFrame = true;
		lastKeyActivity = rs::Timer::now;
	}
	else {
		hasInputKeyThisFrame = false;
	}
}

std::vector<InputEvent> rs::Sys::keyEvents;

class KeyInterpector : public Pasta::ControllerListener {

	virtual void notifyKeyPressed(ControllerType controller, Key key) {
		InputEvent e(InputEventKind::EIK_KeyDown);
		e.origin = rs::InputEvent::resolveOrigin(controller);
		e.relX = -1;
		e.relY = -1;
		e.native = key;
		rs::Sys::keyEvents.push_back(e);
	}

	//beware scanning keyboard here s doomed
	virtual void notifyKeyReleased(ControllerType controller, Key key) {
		rs::InputEvent e(InputEventKind::EIK_KeyUp);
		e.origin = rs::InputEvent::resolveOrigin( controller );
		e.relX = -1;
		e.relY = -1;
		e.native = key;
		rs::Sys::keyEvents.push_back(e);
		//printf("key3 %d", key);
	}

	virtual void onConnexionChange(ControllerType _controller, bool _connected) {


	}

};

void rs::Sys::dispose() {
	//rs::StackTrace:: ?
	//
	enterFrameProcesses.dispose();
	exitFrameProcesses.dispose();
}



void rs::Sys::init() {
	
#ifdef PASTA_WIN
	wchar_t dir[MAX_PATH] = {};
	string strBoot = rd::String::toString(_wgetcwd(dir, MAX_PATH));
	bootDirectory = strBoot.c_str();
#else
	bootDirectory = "";
#endif

	rs::StackTrace::init();
	InputMgr * mgr = InputMgr::getSingleton();
	mgr->addControllerListener(new KeyInterpector());

	if(!r2::GpuObjects::defaultFont)
		r2::GpuObjects::defaultFont = rd::FontManager::get().add("nokia8","fonts/nokia8.fnt", r2::TexFilter::TF_NEAREST, r2::Shader::SH_Basic);
	if (!r2::GpuObjects::defaultFont)
		r2::GpuObjects::defaultFont = rd::FontManager::get().add("nokia8","nokia8.fnt",r2::TexFilter::TF_NEAREST,r2::Shader::SH_Basic);
	if (!r2::GpuObjects::defaultFont) 
		std::cout << "WARNING Cannot find a default font" << std::endl;

	enterFrameProcesses.name = "enterFrameProcesses";
	exitFrameProcesses.name = "exitFrameProcesses";

	rd::Garbage::init();

	new rd::SerializationRegister();
	sysAsserts();
}

unsigned int rs::Sys::uintFromString(const char * val){
	if (strchr(val, '#'))		return strtoul(val + 1, nullptr, 16);
	if (strstr(val, "0x"))		return strtoul(val + 2, nullptr, 16);
	return atoi(val);
}

static uint64_t s_UID = 0;
uint64_t rs::Sys::getUID()
{
	s_UID++;
	if (s_UID == 0) 
		s_UID = 1;
	return s_UID;
}

void rs::Sys::reserveUID(uint64_t uid)
{
	if (s_UID <= uid) 
		s_UID = uid + 1;
}

rd::Tweener* rs::Sys::tw = nullptr;

void rs::Sys::enterFrame(){
	r2::Im::enterFrame();
	rs::Timer::enterFrame();
	rs::Sys::updateInputs();
	enterFrameProcesses.update(rs::Timer::dt);
	rs::Svc::update(rs::Timer::dt);

	if (!tw)
		tw = new rd::Tweener();
	tw->update(rs::Timer::dt);
}

void rs::Sys::exitFrame(){
	exitFrameProcesses.update(rs::Timer::dt);
	rs::Timer::exitFrame();
	r2::SurfacePool::get()->update();
	r2::Im::exitFrame();
	rd::Pools::exitFrame();
}


#ifdef _WIN32
#include <stdio.h>
#include <signal.h>
#define VC_EXTRALEAN
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#endif

namespace rs {

	static std::string signalMessage;
	static bool isInit = false;
	std::function<void(const char*)> Sys::traceOverride = {};

	void abortHandler(int signum) {
		printf("Abortion handler in progress\n");
#ifdef _WIN32
		// associate each signal with a signal name string.

		const char* name = NULL;
		switch (signum) {
		case SIGABRT: name = "SIGABRT";  break;
		case SIGSEGV: name = "SIGSEGV";  break;
			//case SIGBUS:  name = "SIGBUS";   break;
		case SIGILL:  name = "SIGILL";   break;
		case SIGFPE:  name = "SIGFPE";   break;
		}

		// Notify the user which signal was caught. We use printf, because this is the 
		// most basic output function. Once you get a crash, it is possible that more 
		// complex output systems like streams and the like may be corrupted. So we 
		// make the most basic call possible to the lowest level, most 
		// standard print function.

		auto process = [&](const std::string& name, const std::string& s) {
			signalMessage += "<"s+name+">"s + "\n";
			signalMessage += s + "\n";
			signalMessage += "</"s+name+">"s+"\n";
		};

		if (name)
			process("signal_name", name);
		process("signal_num", std::to_string(signum));

		process("game version", rs::StackTrace::versionLabel.c_str());

		if (rd::Console::ALL.size()) 
			for (auto c : rd::Console::ALL) 
				process("log",c->fullLog);
		std::string cs;
		rplatform::getCallStack(cs);
		auto sp = rd::String::split(cs, "stackwalker\\StackWalker.cpp");

		process("timestamp", std::to_string(rs::Timer::getTimeStamp()));
		process("now", std::to_string(rs::Timer::now));
		process("dt", std::to_string(rs::Timer::dt));
		process("dfr", std::to_string(rs::Timer::dfr));

		if( sp.size() >= 2 ){
			process("stack_top", rd::String::replace(sp[1],"\\","/"));
			process("stack_context", rd::String::replace(sp[0], "\\", "/"));
		}
		else 
			process("full_stack",cs);
		
		WCHAR tpath[1024];
		memset(tpath, 0, sizeof(tpath));
		GetTempPathW(sizeof(tpath), tpath);

		WCHAR tpath2[1024];
		memset(tpath2, 0, sizeof(tpath2));
		swprintf_s(tpath2, sizeof(tpath2),L"%s%s",tpath,L"crashdump.xml");
		
		FILE * f = _wfopen(tpath2, L"w");
		fprintf(f, signalMessage.c_str());
		fflush(f);
		fclose(f);


		std::wstring tver = rd::String::toWString(rs::StackTrace::versionLabel.cpp_str() );

		WCHAR msg[2048];
		memset(msg, 0, sizeof(msg));
		wsprintf(msg, L"ver %s\nCrash dump saved at %s, retrieve it using bat/get_crashdump.xml", tver.c_str(), tpath2);
		int msgboxID = MessageBox(NULL, msg, L"The game has encountered an error", MB_ICONERROR | MB_OK);

#endif

		// If you caught one of the above signals, it is likely you just 
		// want to quit your program right now.
		exit(signum);
	}

	Str StackTrace::versionLabel;
	void StackTrace::init()
	{
		if (isInit) return;

#ifdef _WIN32
		signal(SIGABRT, abortHandler);
		signal(SIGSEGV, abortHandler);
		signal(SIGILL, abortHandler);
		signal(SIGFPE, abortHandler);
#endif

		rplatform::Device::initPlatform();
		isInit = true;
	}

	void sysConsolePrint(const char* msg) {
		cout << msg << "\n";
	};

	void trace(const char* msg, int level) {
		if (Sys::LOG_LEVEL >= level)
			trace(msg);
	};

	void trace(const char* msg) {
		if (rd::String::isEmpty(msg))
			return;
		if (rs::Sys::traceOverride)
			rs::Sys::traceOverride(msg);
		else
			cout << Str30f( "%0.2lf %s \n", rs::Timer::stamp(), msg ).c_str();
	};


	static std::unordered_map<Str,bool> once;

	void trace(const string& msg) {
		if (!msg.empty())
			trace(msg.c_str());
	};

	//warning appears only once
	void traceWarning(const char* msg) {
		StrRef msgR(msg);
		if (!rs::Std::exists(once, msg))
			once[msgR] = true;
		else
			return;
		std::string str;
		str += "\033[38;5;208m";
		str += msg;
		str += +"\033[0m";
		trace(str);
	};

	void traceError(const char* msg) {
		std::string str;
		str += "\033[31;5;31m";
		str += msg;
		str += +"\033[0m";
		trace(str);
	};

	void traceGreenlight(const char* msg) {
		std::string str;
		str += "\033[38;5;92m";
		str += msg;
		str += +"\033[0m";
		trace(str);
	};

	void traceObject(const char* prefix, void* obj){
#ifndef PASTA_FINAL
		if (obj) {
			Str256f msg("traceObj: %s addr:0x%x", prefix, obj);
			trace(msg);
		}
#endif 
	}

	void traceAgent(const char* prefix, rd::Agent* ag) {
#ifndef PASTA_FINAL
		if (ag) {
			if (ag->name.length() == 0) {
				int here = 0;
			}
			Str256f msg("traceAg: %s name: %s uid:%ld addr:0x%x", prefix, ag->name.c_str(), ag->_UID, ag);
			trace(msg);
		}
#endif 
	}

	void traceNode(const char* prefix, r2::Node * obj) {
#ifndef PASTA_FINAL
		if (obj) {
			Str256f msg("traceNode: %s name:%s uid:%ld addr:0x%lx", prefix, obj->name.c_str(),obj->uid,obj);
			trace(msg);
		}
#endif 
	}

}

bool rs::Sys::filePickForSave(const std::vector<std::pair<std::string, std::string>> & filters, std::string & result) {
#ifdef PASTA_WIN
	OPENFILENAME ofn = {};
	TCHAR szFile[MAX_PATH] = {};

	HWND hwnd = Pasta::HWNDWindowMgr::getSingleton()->GetGameHWND(0);//TODO
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = hwnd;
	ofn.lpstrFile = szFile;

	// Set lpstrFile[0] to '\0' so that GetOpenFileName does not 
	// use the contents of szFile to initialize itself.
	ofn.lpstrFile[0] = '\0';
	ofn.nMaxFile = sizeof(szFile);

	wstring str;
	for (auto pairs : filters) {
		str += rd::String::toWString(pairs.first) + _T("\0");
		str += rd::String::toWString(pairs.second) + _T("\0");
	}

	ofn.lpstrFilter = str.c_str();

	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	auto bootDirW = rd::String::toWString(rs::Sys::bootDirectory);
	ofn.lpstrInitialDir = bootDirW.c_str();
	if (GetOpenFileName(&ofn) == 1) //holy shit this is long, it basically loads all of windows dll's used in an explorer context
	{
		SetCurrentDirectoryW(bootDirW.c_str());
		wstring wstr(ofn.lpstrFile);
		std::wstring_convert<std::codecvt_utf8<wchar_t>> myconv;
		// convert wstring to UTF-8 string
		result = myconv.to_bytes(wstr);
		return true;
	}
	return false;
#else
	return false;
#endif
}


#include "Checksum.hpp"
r::u32 rs::Sys::CRC32(const std::string& str){
	return Checksum::CRC32(str.data(), str.size());
}

r::u32 rs::Sys::Adler32(const std::string& str) {
	return Checksum::Adler32(str.data(), str.size());
}

void rs::Sys::sysAsserts()
{
}

bool rs::Sys::filePickForOpen(std::string& result) {
	std::vector<std::pair<std::string, std::string>> emptyFilters;
	return rs::Sys::filePickForOpen(emptyFilters, result);
}

bool rs::Sys::filePickForOpen(const std::vector<std::pair<std::string, std::string>> & filters, std::string & result){
#ifdef PASTA_WIN
	OPENFILENAME ofn = {};
	TCHAR szFile[MAX_PATH] = {};

	HWND hwnd = Pasta::HWNDWindowMgr::getSingleton()->GetGameHWND(0);//TODO
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = hwnd;
	ofn.lpstrFile = szFile;

	// Set lpstrFile[0] to '\0' so that GetOpenFileName does not 
	// use the contents of szFile to initialize itself.
	ofn.lpstrFile[0] = '\0';
	ofn.nMaxFile = sizeof(szFile);

	wstring str;
	for (auto pairs : filters) {
		str += rd::String::toWString(pairs.first ) + _T("\0");
		str += rd::String::toWString(pairs.second) + _T("\0");
	}

	ofn.lpstrFilter = str.c_str();

	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = NULL;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
	if (GetOpenFileName(&ofn) == 1) //holy shit this is long, it basically loads all of windows dll's used in an explorer context
	{
		SetCurrentDirectoryW(rd::String::toWString(rs::Sys::bootDirectory).c_str());
		wstring wstr(ofn.lpstrFile);
		std::wstring_convert<std::codecvt_utf8<wchar_t>> myconv;
		// convert wstring to UTF-8 string
		result = myconv.to_bytes(wstr);
		return true;
	}
	return false;
#else
	return false;
#endif
}

/*
void rs::tracef(const char* format, ...) {
	va_list args;
	va_start(args, format);
	trace(Str64f(format, args));
	va_end(args);
};
*/