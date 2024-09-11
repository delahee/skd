#include "CrossPlatform.hpp"

#ifndef PASTA_NX
#pragma message("Exclude this from compile on non NX platforms");
#endif

#ifdef PASTA_NX
std::string rplatform::getAppData() {
	return "";
}

std::string rplatform::getLocalAppData() {
	return "";
}

void shellExecuteOpen(const char*){
	return;
}

void rplatform::getCallStack(std::string& out) {

}
#endif