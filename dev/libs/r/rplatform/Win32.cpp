#include "CrossPlatform.hpp"
#include "rd/String.hpp"

//#define R_USE_STACKWALKER

#ifndef PASTA_WIN
#pragma message("Exclude this from compile on non win32 platforms");
#endif

#ifdef PASTA_WIN

#include <ShlObj.h>
#include <windows.h>
#include <shellapi.h>

std::string rplatform::getAppData() {
	TCHAR szAppData[_MAX_PATH];
	SHGetFolderPath(NULL, CSIDL_APPDATA, NULL, 0, szAppData);
	return rd::String::toString(std::wstring(szAppData));
}

std::string rplatform::getLocalAppData() {
	TCHAR szAppData[_MAX_PATH];
	SHGetFolderPath(NULL, CSIDL_LOCAL_APPDATA, NULL, 0, szAppData);
	return rd::String::toString(std::wstring(szAppData));
}

void rplatform::shellExecuteOpen(const char* path){
	ShellExecuteA(NULL, "open", path, NULL, NULL, SW_SHOWDEFAULT);
}

void rplatform::debugbreak(){
#if PASTA_WIN
	__debugbreak();
#endif
}

#ifdef R_USE_STACKWALKER
#include "StackWalker.h"
//todo get the good dll from windows SDK
class StrStackWalker : public StackWalker {
public:
	std::string buf;
	StrStackWalker() : StackWalker() {}
protected:
	virtual void OnOutput(LPCSTR szText) {
		buf += szText;
		StackWalker::OnOutput(szText);
	};
}; 
#endif
void rplatform::getCallStack(std::string & out){
#ifdef R_USE_STACKWALKER
	auto n = new StrStackWalker();
	n->ShowCallstack();
	out = n->buf;
#endif
}
#endif