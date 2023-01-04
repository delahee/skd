#include "stdafx.h"
#include "File.hpp"

#include <fstream>

#include "1-files/PersistentMgr.h"

using namespace std;

#if defined(PASTA_WIN)
#include <filesystem>

string rs::File::sep()
{
	std::wstring s;
	s += std::filesystem::path::preferred_separator;
	return rd::String::toString(s);
}

bool rs::File::createDir(const std::string& path){
	bool ok = true;
	std::filesystem::path p(path);
	try {
		ok = std::filesystem::create_directory(path);
	}
	catch (...) {
		return false;
	}
	return ok;
}

bool rs::File::exists(const std::string& path, std::string& content) {
	std::filesystem::path p(path);
	return std::filesystem::is_regular_file(path);
}

bool rs::File::read(const std::string& path, std::string& content) {
	std::filesystem::path p(path);
	std::filesystem::path root = p.root_directory();
	if (!std::filesystem::is_directory(root))
		return false;
	FILE* f = fopen(path.c_str(), "rb");//read as binary to avoid fread chunk bhv
	if (!f) return false;
    auto sz = std::filesystem::file_size(p);
	content.resize(sz);
    auto readSz = fread(content.data(), sz, 1, f);
	fclose(f);
	return readSz > 0;
}

bool rs::File::write(const string& path, const std::string& content) {
	std::filesystem::path p(path);
	std::filesystem::path root = p.root_directory();

	if (!root.empty()) {
		if (!std::filesystem::is_directory(root))
			std::filesystem::create_directory(root);
		if (!std::filesystem::is_directory(root))
			return false;
	}
    FILE * f = fopen(path.c_str(), "wb");
    if (!f) return false;
    size_t sz = fwrite(content.data(), content.size(), 1, f) > 0;
    fclose(f);
    return sz > 0;
}
#elif defined(PASTA_NX)
string rs::File::sep() {
	return "/";
}

bool rs::File::createDir(const std::string& path) {
	return false;
}

bool rs::File::read(const std::string& path, std::string& content) {
	return false;
}

bool rs::File::write(const string& path, const std::string& content) {
	return false;
}
#endif

bool rs::File::readSaveFile(const std::string& path, std::string& content) {
	Pasta::PersistentMgr* mgr = Pasta::PersistentMgr::getSingleton();
	void* data = 0;
	int sz = 0;
	mgr->openSave("hbc", Pasta::SaveOpenMode::Read);
		auto err = mgr->readData(path.c_str(), &data, &sz, Pasta::SaveTaskFlags::DisableHeader);
	mgr->closeSave();
	if (err != Pasta::SaveError::NoError)
		return false;
	content = std::string((char*)data, sz);
	mgr->releaseData(data);
	return true;
}

bool rs::File::writeSaveFile(const std::string& path, const std::string& content) {
	Pasta::PersistentMgr* mgr = Pasta::PersistentMgr::getSingleton();
	mgr->openSave("hbc", Pasta::SaveOpenMode::Write);
	auto err = mgr->writeData(path.c_str(), content.data(),content.size() , Pasta::SaveTaskFlags::DisableHeader);
	mgr->closeSave();
	if(err == Pasta::SaveError::NoError)
		return true;
	return true;
}

rs::FileHost::FileHost(const char* _path, std::function<void(const char*)> _onEvalTime, bool doProcessNow) {
	path = _path;
	name = "File Host of " + path;
	onEvalTime = _onEvalTime;
	cpath = Pasta::FileMgr::getSingleton()->convertResourcePath(path);
	if (doProcessNow)
		process();
}

void rs::FileHost::process() {
	Pasta::FileMgr* mgr = Pasta::FileMgr::getSingleton();
	if (onEvalTime) onEvalTime(path.c_str());
	writeTime = mgr->getLastWriteTime(cpath.c_str());
	nbSucessfullProcess++;
}

void rs::FileHost::update(double dt) {
	Pasta::FileMgr* mgr = Pasta::FileMgr::getSingleton();
	rd::Agent::update(dt);
	timer -= dt;
	if (timer < 0) {
		if (mgr->getLastWriteTime(cpath.c_str()) != writeTime)
			process();
		timer = 0.1;
	}
}

rs::FileHost* rs::FileHost::add(const char* _path, std::function<void(const char*)> _onEvalTime){
	auto fh = new FileHost(_path, _onEvalTime, true);
	rs::Svc::reg(fh);
	return fh;
}