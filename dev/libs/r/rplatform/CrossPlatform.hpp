#pragma once

#include <string>

namespace rplatform {
	std::string getAppData();
	std::string getLocalAppData();
	void shellExecuteOpen(const char* path);
	void getCallStack( std::string & out );
	void debugbreak();
}


