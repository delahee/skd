#include "CrossPlatform.hpp"
#include "rd/String.hpp"

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
#endif