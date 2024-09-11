#include "stdafx.h"
#include "PathUtils.hpp"

using namespace rd;
std::string rd::PathUtils::removeAllExtensions(const std::string& p){
	auto pWithoutExt = p.find_first_of('.');
	return p.substr(0, pWithoutExt);
}

std::string PathUtils::removeExtension(const std::string& p){
	auto pWithoutExt = p.find_last_of('.');
	return p.substr(0, pWithoutExt);
}

void PathUtils::removeExtension(Str & p) {
	int idx = rd::String::rfind(p, ".");
	if (idx>=0)
		p[idx] = 0;
}

std::string PathUtils::basename(const std::string& path){
	return removeAllExtensions(path.substr(path.find_last_of("/\\") + 1));
}

void rd::PathUtils::listDir(const char * folder, const char* ext, std::vector<std::string>& res){
	res.clear();
	Pasta::FileMgr* mgr = Pasta::FileMgr::getSingleton();
	Pasta::DirData* dir = mgr->openDir(folder);
	int sufLen = strlen(ext);
	while (true) {
		const Pasta::DirEntry* entry = mgr->nextDirEntry(dir, Pasta::FileMgr::LIST_FILES);
		if (!entry) break;
		if ( rd::String::endsWith( entry->name , ext))
			res.push_back(entry->name);
	}
	eastl::stable_sort(res.begin(), res.end());
	mgr->closeDir(dir);
}


void rd::PathUtils::listDir(const char* folder, const char* ext, eastl::vector<Str>& res) {
	res.clear();
	Pasta::FileMgr* mgr = Pasta::FileMgr::getSingleton();
	Pasta::DirData* dir = mgr->openDir(folder);
	int sufLen = strlen(ext);
	while (true) {
		const Pasta::DirEntry* entry = mgr->nextDirEntry(dir, Pasta::FileMgr::LIST_FILES);
		if (!entry) break;
		if ( rd::String::endsWithI( entry->name.c_str(), ext))
			res.push_back(entry->name);
	}
	eastl::stable_sort(res.begin(), res.end());
	mgr->closeDir(dir);
}