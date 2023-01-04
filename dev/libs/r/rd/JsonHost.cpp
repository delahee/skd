#include "stdafx.h"

#include <string>
#include <algorithm>
#include <iostream>

#include "JsonHost.hpp"
#include "1-files/FileMgr.h"
#include "1-json/jsoncpp/json.h"
#include "1-json/jsoncpp/reader.h"

#include "r2/all.hpp"

using namespace std;
using namespace rd;
using namespace Pasta;
using namespace Json;

#define SUPER Agent

double rd::JsonHost::getFloat(const char * label, const char * param)
{
	Value & v = all[label]; 
	Value p = v.get(param, ValueType::realValue);
	return p.asDouble();
}

JsonHost::JsonHost(std::string & scriptPath, AgentList * _al) : SUPER(_al) {
	FileMgr * mgr = FileMgr::getSingleton();
	file = mgr->createFile(filePath = scriptPath);

	if (!file && debug) {
		printf("Warn:Json::No such file %s\n", scriptPath.c_str());
	}
}

void rd::JsonHost::cleanRsc() {
	if (root.data) {
		delete(root.root);
		root.root = nullptr;
	}
}

rd::JsonHost::~JsonHost() {
	cleanRsc();
}

void JsonHost::eval() {
	FileMgr * mgr = FileMgr::getSingleton();

	file->load();
	char * rawContent = (char*)file->getContent();
	if (rawContent == nullptr)
		return;

	try {
		init = true;
		
		root.data = rawContent;

		Json::Reader rd(Json::Features::all());

		Json::Value * rv = new Json::Value(Json::ValueType::objectValue);
		rd.parse(std::string((char*)rawContent ), *rv);

		root.root = rv;

		all.clear();
		
		Value v = rv->get("all", ValueType::objectValue);
		
		for (Value vs : v) {
			string name = vs["name"].asString();
			all[name] = vs;
		}
	}
	catch (std::exception &e) {
		std::cout << "Std Error\n" << e.what() << '\n';
	}
	root.data = nullptr;
	file->release();
	lastReadTime = mgr->getLastWriteTime(file->getConvertedPath());
}

void JsonHost::update(double dt) {
	checkProgress += dt;
	if (checkProgress >= DELTA_CHECK) {
		FileMgr * mgr = FileMgr::getSingleton();
		Pasta::u64 writeTime = mgr->getLastWriteTime(file->getConvertedPath());
		if (writeTime > lastReadTime) {
			cleanRsc();
			eval();
		}
		checkProgress = 0.0f;
	}
}
#undef SUPER

