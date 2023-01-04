#include "stdafx.h"

#include "Str.h"

#include "1-texts/TextMgr.h"
#include "1-files/FileMgr.h"

#include "Tsv.hpp"


using namespace std;
using namespace fmt;

vector<std::string> Tsv::parse(std::string content, char sep) {
	std::vector<std::string> vec;
	vec.clear();

	string::size_type stTemp = content.find(sep);

	while (stTemp != string::npos)
	{
		vec.push_back(content.substr(0, stTemp));
		content = content.substr(stTemp + 1);
		stTemp = content.find(sep);
	}

	vec.push_back(content);

	return vec;
}

std::vector<std::string> Tsv::extractLines(const char * file)
{
	string stdFile(file);
	while (Pasta::TextMgr::replace(stdFile, "\r", ""));
	return parse(stdFile.c_str(), '\n');
}

vector<std::string> Tsv::extractColumns(const char * line)
{
	return parse(line, '\t');
}

class ParseTSVHost : public rd::Agent {
public:
	double timer = 0;
	Pasta::u64 writeTime = 0;
	string path;
	string cpath;
	std::function<void(vector<string>& _prm)> assignment;

	ParseTSVHost(const char* _path, std::function<void(vector<string>& _prm)> _assignment) {
		path = _path;
		cpath = Pasta::FileMgr::getSingleton()->convertResourcePath(path);
		assignment = _assignment;
		process();
	}

	void process() {
		Pasta::FileMgr* mgr = Pasta::FileMgr::getSingleton();
		Tsv::parseAndAssign(path, assignment);//it seems
		writeTime = mgr->getLastWriteTime(cpath.c_str());
	}

	void update(double dt)override {
		Pasta::FileMgr* mgr = Pasta::FileMgr::getSingleton();
		Agent::update(dt);
		timer -= dt;
		if (timer < 0) {
			if (mgr->getLastWriteTime(cpath.c_str()) != writeTime)
				process();
			timer = 0.1;
		}
	}
};

void Tsv::watchParseAndAssign(const char* path, std::function<void(vector<string>& _prm)> assign) {
#ifdef PASTA_FINAL
//#if 1
	parseAndAssign(path, assign);
#else 
	ParseTSVHost* f = new ParseTSVHost(path, assign);
	f->name = string("TSV Host of ") + path;
	rs::Svc::reg(f);
#endif
}

void Tsv::parseAndAssign(const string& path, std::function<void(vector<string>& _prm)> assign) {
	parseAndAssign(path.c_str(),assign);
}

void Tsv::parseAndAssign(const char* path, std::function<void(vector<string> & _prm)> assign) {
	char * content = rd::RscLib::getText(path);
	if (content == nullptr)
	{
#ifdef _DEBUG
		cout << "no such tsv file ? " << path << endl;
#endif
		return;
	}

	vector<string> lines = Tsv::extractLines(content);
	if (lines.size() <= 1) {
		rd::RscLib::releaseText(content);
		return;
	}

	for (string & l : lines) {
		std::vector<string> columns = Tsv::extractColumns(l.c_str());
		assign(columns);
	}

	rd::RscLib::releaseText(content);
}

std::vector<Str> Tsv::parseIdentifierList(vector<std::string>& line, int idx) {
	if (idx < 0) return {};
	const char* str = line[idx].c_str();
	return rd::String::parseIdentifierList(str);
}

std::vector<Str> Tsv::parseIdentifierList(vector<Str>& line, int idx) {
	if (idx < 0) return {};
	const char * str = line[idx].c_str();
	return rd::String::parseIdentifierList(str);
}

EnumList Tsv::parseEnums(std::vector<std::string>& line, int idx){
	if (idx < 0) return {};
	const char* str = line[idx].c_str();
	return rd::Enum::parseEnumList(str,',');
}

rd::Enum Tsv::parseEnum(std::vector<std::string>& line, int idx) {
	if (idx < 0) return {};
	const char* str = line[idx].c_str();
	return rd::Enum::parseEnum(str);
}

rd::Vars* Tsv::parseVars(const Str& src){
	//could split  with commas first
	rd::Vars v;

	auto an = v.getOrCreate("val");
	rd::AType at = rd::AType::AInt;

	if (rd::String::isBoolean(src.c_str())) {
		at = rd::AType::AInt;
		an->setValueFromString(at, rd::ATypeEx::ABool, src.c_str());
		//shortcut a bit
		return new rd::Vars(v);
	}
	else if (rd::String::isInteger(src.c_str()))
		at = rd::AType::AInt;
	else if(rd::String::isFloat(src.c_str()))
		at = rd::AType::AFloat;
	else 
		at = rd::AType::AString;

	an->setValueFromString(at, src.c_str());

	return new rd::Vars(v);
}