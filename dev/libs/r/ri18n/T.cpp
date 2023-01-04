#include "stdafx.h"

#include "1-texts/TextMgr.h"
#include "Str.h"

#include "T.hpp"

#include "fmt/Tsv.hpp"
#include "rd/Vars.hpp"
#include "rs/Std.hpp"
#include "rs/File.hpp"

using namespace std;
using namespace ri18n;
using namespace Pasta;

static std::vector<std::string> langs = { 
	std::string("en"),
	//string("fr"), 
	//string("jp") 
};

std::unordered_map<Str, KeyText>				T::uiFiles;
std::unordered_map<Str, shared_ptr<DialogFile>>	T::dialFiles;
std::string										T::curLang;
std::string										T::dummyStdStr = "";
Str												T::dummyStr = "";
Sig												T::onLanguageChanged;
bool											T::dumpMiss = true;

std::unordered_map<string, int>					T::tmp_lang_idx;
std::unordered_map<int, string>					T::tmp_idx_lang;
bool											T::tmp_do_process = false;

static std::string								s_TempKey;
static std::string								s_TempQuery;
static Str										s_emptyStr;

std::function<void(int, rd::Font*)>		T::onUnknownFontCharacterEncountered;
const char* T::euroChars = u8"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz '-&quot;'%,?!$+:*.éèêëàäâïîüç0123456789+=><()[]/âàäéèêëîïôöùûüçáéíóúÁÉÍÓÚÑñ¡¿äÄÖÜß_²αβ";

bool T::imOpened = true;

void T::init() {
	init("en");
}

void ri18n::T::imWindowed(){
	using namespace ImGui;
	if (Begin("I18n / Texts", &imOpened)) 
		T::im();
	End();
}

void T::im() {
	using namespace ImGui;
	if (TreeNode("Dialog Files")) {
		for (auto& dfp : dialFiles) {
			PushID(dfp.first.c_str());
			LabelText("file", dfp.first.c_str());
			if (TreeNode("Content")) {
				for (auto& l : dfp.second->lines) {
					PushID(&l);
					if (ImGui::TreeNode((string("id:") + l.id.c_str()).c_str())) {
						Indent();
						for (auto& s : l.texts)
							ImGui::Text("%s -> %s", s.first.c_str(), s.second.c_str());
						Unindent();
						TreePop();
					}
					if (IsItemHovered()) {
						string t;
						t += "threadId:";
						t += l.threadId;
						SetTooltip(t.c_str());
					}
					PopID();
				}
				TreePop();
			}
			PopID();
		}
		TreePop();
	}

	if (TreeNode("UI Files")) {
		for (auto& ui : uiFiles) {
			PushID(ui.first.c_str());
			LabelText("id", ui.first.c_str());
			Indent();
			for (auto& s : ui.second.kv)
				ImGui::Text("%s -> %s", s.first.c_str(), s.second.c_str());
			Unindent();
			PopID();
		}
		TreePop();
	}

	if (TreeNode("Query")){
		auto backupMiss = dumpMiss;
		dumpMiss = false;
		InputText("input",s_TempQuery);
		if (s_TempQuery.length()) {
			auto hasQuery = has(s_TempQuery.c_str());
			Value("Result for has():", hasQuery);
			auto getQuery = get(s_TempQuery.c_str());
			Text("Result for get(): %s", getQuery.c_str());
		}
		TreePop();
		dumpMiss = backupMiss;
	}
}



void T::init(const string& defaultLang){
	curLang = defaultLang;

#ifndef PASTA_NX
	std::setlocale(LC_ALL, "en_US.UTF-8");
#endif

	char * lang = Pasta::TextMgr::getSingleton()->getLocalization();
	for (string & lg : langs) {
		if( lg == lang){
			curLang = lang;
			break;
		}
	}
	onLanguageChanged.trigger();
	delete [] lang;
	cout << "Main language set to " << curLang << endl;
}

void T::changeLang(const string& lang) {
	for (auto& availableLang : langs) {
		if (lang == availableLang) {
			curLang = lang;
			cout << "Main language set to " << curLang << endl;
			return;
		}
	}
	cout << "Language " << lang << " not found" << endl;
	onLanguageChanged.trigger();
}

void T::init(vector<string>& _langs){
	langs = _langs;
}

void T::destroy(){
	curLang="";
	uiFiles.clear();
	dialFiles.clear();
}

const Str& ri18n::T::getStr(const Str& key){
	return getStr(key.c_str());
}

const Str& T::getStr(const string& key) {
	return getStr(key.data());
}

const char* T::getCStr(const string& key) {
	return getCStr(key.c_str());
}

static const char* TEXT_ERR_TAG = "~";
const Str& T::getStr(const char* key) {
	ri18n::KeyText* file = rs::Std::get(uiFiles, key);
	if (!file) {
		s_TempKey = tolower(key);
		file = rs::Std::get(uiFiles, s_TempKey.data());
	}
	if (!file) {
		if(dumpMiss) trace(std::string() + "Unknown key in uiFiles #" + key);
		dummyStr.clear();
		dummyStr = string(TEXT_ERR_TAG) + key;
		return dummyStr;
	}
	Str* s = rs::Std::get(file->kv, StrRef(curLang));
	if (!s) {
		dummyStr.clear();
		dummyStr = string(TEXT_ERR_TAG)+key;
		return dummyStr;
	}
	return *s;
}

const char * T::getCStr(const char* key) {
	return getStr(key).c_str();
}

bool T::has(const char* key) {
	ri18n::KeyText* file = rs::Std::get(uiFiles, key);
	if (!file) {
		s_TempKey = tolower(key);
		file = rs::Std::get(uiFiles, s_TempKey.data());
	}
	if (!file) 
		return false;
	Str* s = rs::Std::get(file->kv, StrRef(curLang));
	if (!s)
		return false;
	return true;
}

std::string T::get(const std::string& key) {
	return get(key.c_str());
}

std::string T::get(const char* key) {
	return std::string(getStr(key).c_str());
}

std::string T::getU(const char * key){
	const Str& s = getStr(key);
	return toupper( s.cpp_str() );
}

Str T::getStrU(const char* key) {
	return Str(getU(key));
}

SheetLine * T::getDialogLine(const char * key, const char * threadId, shared_ptr<DialogFile> sheet) {
	if (threadId == nullptr) threadId = "0";
	
	auto & id2Line = sheet->id2Line;
	if (id2Line.find(key) == id2Line.end()) 
		return nullptr;

	if (strcmp(threadId, "0") == 0) 
		return &(sheet->lines[id2Line[key]]);
	
	auto first = sheet->lines.begin() + id2Line[key];
	while( first->threadId != threadId ) {
		first++;
		if (first->id != key)
			return nullptr;
	}
	return &(*first);
}

SheetLine * T::getDialogLine(const char * key, const char * threadId, const char * sheet)
{
	if (threadId == nullptr) threadId = "0";
	if (sheet == nullptr) {
		for (auto & dfp : dialFiles) {
			SheetLine * dl = getDialogLine(key, threadId, dfp.second);
			if (dl != nullptr)
				return dl;
		}
	}
	else {
		if (dialFiles.find(sheet) == dialFiles.end())
			return nullptr;
		auto df = dialFiles[sheet];
		return getDialogLine(key, threadId, df);
	}
	return nullptr;
}

vector<SheetLine*> 
T::getThread(const char * key, const char * threadId, const char * sheet)
{
	if (threadId == nullptr) threadId = "0";
	if (sheet == nullptr) {
		for (auto dfp : dialFiles) {
			if (dfp.second->id2Line.find(key) != dfp.second->id2Line.end())
				return getThread(key, threadId, dfp.second);
		}
	}
	else {
		if (dialFiles.find(sheet) == dialFiles.end())
			return vector<SheetLine*>();
		auto df = dialFiles[sheet];
		return getThread(key, threadId, df);
	}
	return vector<SheetLine*>();
}

vector<SheetLine*> T::getThread(const char * key, const char * threadId, shared_ptr<DialogFile> sheet)
{
	if (threadId == nullptr) threadId = "0";
	vector<SheetLine*> res;
	
	auto start = sheet->lines.begin() + sheet->id2Line[key];
	auto next = start;
	const string & id = start->id;
	while (id == next->id) {
		if( next->threadId == threadId )
			res.push_back(&(*next));
		next++;
	}
	return res;
}

vector<SheetLine*> 
T::getDialogs(const char * key, const char * threadId, const char * sheet)
{
	if (threadId == nullptr) threadId = "0";
	if (sheet == nullptr) {
		for (auto dfp : dialFiles) {
			if (dfp.second->id2Line.find(key) != dfp.second->id2Line.end())
				return getDialogs(key, threadId, dfp.second);
		}
	}
	else {
		if (dialFiles.find(sheet) == dialFiles.end())
			return vector<SheetLine*>();
		auto df = dialFiles[sheet];
		return getDialogs(key, threadId, df);
	}
	return vector<SheetLine*>();
}

vector<SheetLine*>
T::getDialogs(const char * key, const char * threadId, shared_ptr<DialogFile> sheet)
{
	if (threadId == nullptr) threadId = "0";
	vector<SheetLine*> res;
	auto start = sheet->lines.begin() + sheet->id2Line[key];
	auto next = start;
	const string & id = start->id;
	while (id == next->id) {
		res.push_back(&(*next));
		next++;
	}
	return res;
}

void T::clearUiTexts(){
	uiFiles.clear();
}


shared_ptr<DialogFile> T::loadThreadedFile(const char * filename ){
	int _line = 0;
	bool doProcess = true;

	unordered_map<string, int> lang_idx;
	unordered_map<int, string> idx_lang;

	int key_idx = 0;
	int char_idx = -1;
	int thread_idx = -1;
	int cond_idx = -1;

	string prevId = "";
	int curAutoThreadId = 0;
	string threadId = "";
	string id = "";

	shared_ptr< DialogFile> df = make_shared<DialogFile>();

	vector<SheetLine> allString;
	unordered_map<string, int> id2Line;
	string empty="";

	std::function<void(vector<string>& line)> parse = [&](vector<string> & line) {
		if (!doProcess)return;

		if (_line == 0) {
			auto b = line.begin();
			auto e = line.end();
			int idx = 0;
			for (string & l : line) {
				if (l == "id") key_idx = idx;
				else if (l == "key") key_idx = idx;
				else if (l == "char" ||  l == "character" || l == "Characters")
					char_idx = idx;
				else if (l == "thread") thread_idx = idx;
				else if (l == "cond") cond_idx = idx;
				else {
					bool found = false;
					for (string & lg : langs) {
						if (lg == l) {
							lang_idx[lg] = idx;
							idx_lang[idx] = lg;
							found = true;
							break;
						}
					}
					if (!found && l != string("")
						&& l != "images"
						&& l != "word"
						&& l != "count EN"
						&& l != "notes"
						&& l != "maxchars"
						)
						cout << "Unrecognised column key ? " << l << endl;
				}

				idx++;
			}
		}
		else {
			
			bool isNewId = prevId == id;

			string charName = empty;

			if (isNewId) 
				curAutoThreadId = 0;

			if (char_idx == -1) 
				cout << "unable to find character column" << endl;
			else 
				charName = line[char_idx];

			const string & key = line[key_idx];
			unordered_map<Str, Str> texts;
			for (auto& p : lang_idx) 
				texts[p.first] = line[p.second];
			threadId = line[thread_idx];
			bool isAutoThread = (threadId == "");

			prevId = id;
			id = key;

			SheetLine t(
				id,
				(isAutoThread ? to_string(curAutoThreadId * 100) : threadId),
				charName,
				texts,
				( cond_idx>=0 ) ? line[cond_idx] : empty
			);
			allString.push_back(t);
			if (isNewId)
				id2Line[id] = allString.size() - 1;
		}
		_line++;
	};
	fmt::Tsv::parseAndAssign(filename, parse);
	std::swap(df->lines, allString);
	std::swap(df->id2Line, id2Line);
	dialFiles[filename] = df;
	return df;
}

void T::loadUiFile(const char * filename){
	std::function<void(std::vector<string>&)> parse = [](std::vector<std::string>& line) {
		if (line[0] == "id") {//found an id line start processing
			tmp_lang_idx.clear();
			tmp_idx_lang.clear();
			tmp_do_process = true;

			auto b = line.begin();
			auto e = line.end();
			int idx = 0;
			for (string& l : line) {
				bool found = false;
				for (string& lg : langs) {
					if (lg == l) {
						tmp_lang_idx[lg] = idx;
						tmp_idx_lang[idx] = lg;
						found = true;
						break;
					}
				}
				if (!found && l != string("")
					&& l != "id"
					&& l != "images"
					&& l != "word"
					&& l != "count EN"
					&& l != "notes"
					&& l != "maxchars"
					) {
					cout << "Unrecognised lang ? " << l << endl;
				}
				idx++;
			}
		}
		else if(tmp_do_process) {
			for (int i = 0; i < line.size(); ++i) {
				if (tmp_idx_lang.find(i) != tmp_idx_lang.end()) {
					s_TempKey = tolower(line[0]);
					uiFiles[line[0]].kv[tmp_idx_lang[i]] = line[i];
					if (s_TempKey != line[0])
						uiFiles[s_TempKey].kv[tmp_idx_lang[i]] = line[i];
				}
			}

			if (line[0] == "FIN" || line[0] == "EOF" || line[0] == "END") {
				tmp_do_process = false;
			}
		}
	};

	fmt::Tsv::watchParseAndAssign(filename, parse );
}

const char * utf8EEszet = u8"ẞ";
const char * eszet = "ß";
const char * eszetMaj = "SS";

string T::toupper(const string& str) {
	string res = str;
	size_t eszetPos = res.find(utf8EEszet);
	while (eszetPos != std::string::npos) {
		res.replace(eszetPos, strlen(utf8EEszet), eszetMaj);
		eszetPos = res.find(utf8EEszet);
	}
	eszetPos = res.find(eszet);
	while (eszetPos != std::string::npos) {
		res.replace(eszetPos, strlen(eszet), eszetMaj);
		eszetPos = res.find(eszet);
	}
	std::transform(res.begin(), res.end(), res.begin(), ::toupper);
	return res;
}

string T::toupper(const char* str) {
	return toupper(std::string(str));
};

Str T::toupper(const Str& str) {
	return Str(str.cpp_str());
};

std::string T::tolower(const char* str){
	string res = str;
	if (curLang == "de") {
		auto eszetPos = res.find(eszetMaj);
		while (eszetPos != std::string::npos) {
			res.replace(eszetPos, strlen(eszetMaj), eszet);
			eszetPos = res.find(eszetMaj);
		}
	}
	std::transform(res.begin(), res.end(), res.begin(), ::tolower);
	return res;
}

Str T::tolower(Str& str){
	Str res; 
	res = tolower(str.c_str());
	return res;
}

std::string T::tolower(const string& str) {
	return tolower(str.c_str());
}

shared_ptr<DialogFile> T::getDialogSheet(const char * name){
	if (dialFiles.find(name) == dialFiles.end())
		return nullptr;
	return dialFiles[name];
}

void T::testAST(const char * filename){
	vector<shared_ptr<DialogFile>> dials;

	for (const string & lg : langs) {
		shared_ptr<DialogFile> file = loadThreadedFile(filename);
		dials.push_back(file);
		compileDialogFile(file);
	}
	
	shared_ptr<DialogFile> ref = dials[0];
	Nar n;
	int nLine = 0;
	for (auto l : ref->lines) {
		for (int i = 1; i < dials.size(); ++i) {
			SheetLine & lref = (ref->lines[nLine]);
			SheetLine & l0 = (dials[i]->lines[nLine]);
			bool okw = debugWarningCompareAst(lref.ast, l0.ast);
			if (!okw) {
				cout << "line WARNING: " << nLine << endl;
				cout << "AST Mismatch"<<endl;
				cout << "#0 " << n.stringify(lref.ast) << endl;
				cout << "> #1 " << n.stringify(l0.ast) << endl;
				cout << "##############" << endl << endl;
			}
			else {
				//cout << "nline ok: " << nLine << endl;
			}
			bool oke = debugErrorCompareAst(lref.ast, l0.ast);
			if (!oke) {
				cout << "nline ERROR: " << nLine << endl;
				cout << "AST Mismatch" << endl;
				cout << "#0 " << n.stringify(lref.ast) << endl;
				cout << "> #1 " << n.stringify(l0.ast) << endl;
				cout << "##############" << endl << endl;
			}
			else {
				//cout << "nline ok: " << nLine << endl;
			}
		}
		nLine++;
	}
}

void T::compileDialogFile(shared_ptr<DialogFile> f)
{
	Nar n;
	for (auto & lines : f->lines) {
		Str* res = rs::Std::get(lines.texts, StrRef(curLang));
		//lines.ast = n.make( );
	}
}

bool T::debugWarningCompareAst(AstNode * ref, AstNode * tgt)
{
	if (ref == nullptr && tgt == nullptr) return true;

	bool resp = true;
	
	vector<AstNodeType> comps = { AstNodeType::CondEvent,AstNodeType::Event,AstNodeType::TagFrom,AstNodeType::Tag };
		
	for (AstNodeType ant : comps) {
		int countRef = (ref == nullptr) ? 0 : ref->countType(ant);
		int countTgt = (tgt == nullptr) ? 0 : tgt->countType(ant);
		if (countRef != countTgt) {
			cout << "event count mismatch for event type "<< AstNode::to_string(ant) << " !" << endl;
			resp = false;
		}
	}
	return resp;

	//then compare scripts
}

bool T::debugErrorCompareAst(AstNode * ref, AstNode * tgt)
{
	if (ref == nullptr && tgt == nullptr) return true;

	bool resp = true;
	
	vector<AstNodeType> comps = { AstNodeType::Script,AstNodeType::ImportantEvent,AstNodeType::CondUniqueEvent };

	for (AstNodeType ant : comps) {
		int countRef = (ref == nullptr) ? 0 : ref->countType(ant);
		int countTgt = (tgt == nullptr) ? 0 : tgt->countType(ant);
		if (countRef != countTgt) {
			cout << "Sys event count mismatch for event type " << AstNode::to_string(ant) << " !" << endl;
			cout << to_string(countRef) << "<>" << to_string(countTgt) << endl;
			resp = false;
		}
	}

	return resp;

	//then compare scripts
}


bool T::emitCharsets(vector<string> langs, const char* out) {
	rd::BitArray charset;
	vector<int> txtCodePoints;
	for (auto& lang : langs) {
		changeLang(lang);

		for (auto& entry : uiFiles){
			std::string str = get(entry.first.c_str());
			rd::String::toCodePoints(str.c_str(), txtCodePoints);
			for (auto& codePt : txtCodePoints)
				charset.set(codePt);
		}
		for (auto& entry : dialFiles) {
			for (auto& line : entry.second->lines) {
				auto& str = line.texts[lang];
				rd::String::toCodePoints(str.c_str(), txtCodePoints);
				for (auto& codePt : txtCodePoints)
					charset.set(codePt);
			}
		}
	}
	string content = "";
	char utf8[5] = {};
	int idx = 0;
	for (uint32_t code = 1; code < charset.getMaxIdx(); ++code) {
		if (charset.is(code)){
			memset(utf8, 0, 5);
			idx = 0;
			Pasta::TextMgr::getSingleton()->encodeUTF8((int)code, utf8, &idx);
			content += utf8;
		}
	}
	rs::File::write(out, content);
	return true;
}

vector<char> ri18n::T::getSoftPunctuation(){
	vector<char> punct;

	if (   curLang == fr
		|| curLang == en
		|| curLang == es
		|| curLang == it
		|| curLang == de){
		punct.push_back(',');
		punct.push_back(':');
	}

	punct.push_back(' ');
	return punct;
}

vector<char> ri18n::T::getHardPunctuation()
{
	vector<char> punct;

	if (curLang == fr
		|| curLang == en
		|| curLang == es
		|| curLang == it
		|| curLang == de) {
		punct.push_back('.');
		punct.push_back(';');
		punct.push_back('!');
		punct.push_back('?');
	}

	punct.push_back('\n');
	return punct;
}

std::string ri18n::T::firstCapitalize(const char* str){
	std::string label = str;
	int pos = 0;
	int charCode = TextMgr::decode1UTF8(label.data(), &pos);
	label.erase(label.begin(), label.begin() + pos);
	std::string c = rd::String::c32toUtf8(&charCode, 1);
	c = toupper(c);
	return c + label;
}

void ri18n::T::firstCapitalize(std::string& label){
	if (label.length() == 0) return;
	int pos = 0;
	int charCode = TextMgr::decode1UTF8(label.data(), &pos);
	label.erase(label.begin(), label.begin() + pos);
	std::string c = rd::String::c32toUtf8(&charCode, 1);
	c = toupper(c);
	label = c + label;
}

std::string ri18n::T::format(const char* str, std::initializer_list<std::pair<const char*, const char*>> mp){
	std::string s = str;
	for (auto& p : mp) {
		if ( !rd::String::contains(p.first, p.second) == 0) continue;
		std::string ss = rd::String::replace(s, p.first, p.second);
		s = ss;
	}
	return s;
}

std::string ri18n::T::format(const char* str, std::unordered_map<const char*,const char *> &mp){
	std::string s = str;
	for(auto & p : mp){
		std::string ss = rd::String::replace(s, p.first, p.second);
		s = ss;
	}
	return s;
}

std::string ri18n::T::format(const char* str, std::unordered_map<std::string, std::string>& mp) {
	std::string s = str;
	for (auto& p : mp) {
		std::string ss = rd::String::replace(s, p.first, p.second);
		s = ss;
	}
	return s;
}

std::string ri18n::T::format(const char* str, std::initializer_list<std::pair<std::string, std::string>> mp){
	std::string s = str;
	for (auto& p : mp) {
		std::string ss = rd::String::replace(s, p.first, p.second);
		s = ss;
	}
	return s;
}

std::string ri18n::T::gameplayNumber(float f){
	if( r::Math::isFloat(f ))
		return to_string(f);
	else 
		return to_string(std::lrint(f));
}

std::string TF(const char* id, const char* p0) {
	return ri18n::T::format(id, { std::make_pair("$0",p0) });
}

std::string TF(const char* id, const char* p0, const char* p1) {
	return ri18n::T::format(id, { std::make_pair("$0",p0),std::make_pair("$1",p1) });
}

std::string TF(const char* id, const std::string& p0) {
	return TF(id, p0.c_str());
}

std::string TF(const char* id, const std::string& p0, const std::string& p1) {
	return TF(id, p0.c_str(),p1.c_str());
}

std::string TF(const char* id, int p0) {
	return TF(id, std::to_string(p0));
}

std::string TF(const char* id, int p0,int p1) {
	return TF(id, std::to_string(p0).c_str(), std::to_string(p1).c_str());
}

