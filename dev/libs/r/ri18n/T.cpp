#include "stdafx.h"

#include "T.hpp"

#include <iostream>
#include <ios>

#include "fmt/Tsv.hpp"
#include "rd/Vars.hpp"
#include "rs/Std.hpp"
#include "rs/File.hpp"

#include "utf8.h"

#include "Nar.hpp"
#include "RandText.hpp"
#include "rd/parse/Conds.hpp"

using namespace std;
using namespace std::literals;
using namespace rs;
using namespace rd;
using namespace ri18n;
using namespace Pasta;

static std::vector<std::string> langs = { 
	"en",
	"fr", 
	//string("jp") 
};

#define nbsp " "

static std::vector<std::pair<const char*, const char*>> s_rps = {
	std::make_pair("î", "i"),
	std::make_pair(u8"î", "i"),

	std::make_pair("/n ", "\n"),

	std::make_pair("Ç", "C"),
	std::make_pair(u8"Ç", "C"),
	std::make_pair(u8"À", "A"),
	std::make_pair(u8"É", "E"),
	std::make_pair(u8"È", "E"),
	std::make_pair(u8"œ", "oe"),
	std::make_pair(u8"œ", "oe"),
	
	std::make_pair(" !", nbsp "!"),
	std::make_pair(" ?", nbsp "?"),

	std::make_pair("’", "'"),
	std::make_pair(u8"’", "'"),
	std::make_pair("`", "'"),
	std::make_pair("…", "..."),
	std::make_pair(u8"…", "..."),

	std::make_pair(u8"«", u8"\""),
	std::make_pair(u8"«", u8"\""),
	std::make_pair(u8"»", u8"\""),
	std::make_pair(u8"“", u8"\""),
	std::make_pair(u8"”", u8"\""),
	std::make_pair(u8"‘", u8"'"),
	std::make_pair(u8"’", u8"'"),
	std::make_pair(u8"´", u8"'"),
	std::make_pair(u8"‘", u8"'"),
	std::make_pair(u8"‹", u8"<"),
	std::make_pair(u8"›", u8">"),

	std::make_pair("/n", "\n"),
	std::make_pair("\\n ", "\n"),
	std::make_pair("\\n", "\n"),
	std::make_pair("\\n", "\n"),
	std::make_pair("<br/>", "\n"),
	std::make_pair("&nbsp", nbsp),
};
#undef nbsp

std::unordered_map<Str, KeyText>				T::uiFiles;
std::unordered_map<Str, shared_ptr<DialogFile>>	T::dialFiles;
std::unordered_map<Str, bool>					T::uiOrphans;
std::unordered_map<Str, bool>					T::dialogOrphans;
std::string										T::curLang;
std::string										T::DEFAULT_LANG = "en";
std::string										T::dummyStdStr = "";
std::string										s_emptyCppStr = "";
Str												T::dummyStr = "";
Str												T::emptyStr = "";
Str												T::procGenStr = "";
rd::Sig											T::onLanguageChanged;

bool											T::dumpMiss =
	#if PASTA_DEBUG
	true;
	#else
	false;
	#endif

std::unordered_map<std::string, int>			T::tmp_lang_idx;
std::unordered_map<int, std::string>			T::tmp_idx_lang;
bool											T::tmp_do_process = false;

static Str										tuStr = "";

static std::string								s_TempKey;
static std::string								s_TempQuery;
static Str										s_emptyStr;
bool											T::ALLOW_PROCGEN_INJECTION = true;
std::vector<ITextprocessor*>					T::processors;
rd::parse::CondContext*							T::triggerContext{};

std::function<void(int, rd::Font*)>		T::onUnknownFontCharacterEncountered;
const char* T::euroChars = u8"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz '-&quot;'%,?!$+:*.éèêëàäâïîüç0123456789+=><()[]/âàäéèêëîïôöùûüçáéíóúÁÉÍÓÚÑñ¡¿äÄÖÜß_²αβ€";

static std::unordered_map<string, string> s_up = {
	{u8"ẞ",u8"SS"},
	/*
	{u8"è",u8"È"},
	{u8"é",u8"É"},
	{u8"ê",u8"Ê"},
	
	{u8"ë",u8"Ë"},
	{u8"ë",u8"Ë"},
	{u8"à",u8"À" },
	{u8"ä",u8"Ä" },
	{u8"ô",u8"Ô" },
	*/
};

bool T::imOpened = true;
bool T::isInit = false;
bool T::initialSystemLangFetched = false;


void T::init() {
	if (isInit)
		return;
	init("en");
}

void ri18n::T::imWindowed(){
	using namespace ImGui;
	if (Begin("I18n / Texts", &imOpened)) 
		T::im();
	End();
}

eastl::vector<std::string> ri18n::T::listLangs() {
	return rs::Std::eastlArray( langs );
}

static std::string	dialQuery;
static std::string	orphans;
static Str			checkedLang = "en";

void T::imTranslationCheck() {
	using namespace ImGui;

	Value("current lang", curLang);
	if (BeginCombo("Checked lang", checkedLang.c_str())) {
		for (auto& l : langs)
			if (Selectable(l.c_str(), (checkedLang == l)))
				checkedLang = l;
		EndCombo();
	}


	if (uiOrphans.size() ) {
		if (TreeNode("UI orphans")) {
			for (auto& kv : uiOrphans)
				Text(kv.first);
			TreePop();
		}
	}

	if (dialogOrphans.size()) {
		if (TreeNode("Dialog orphans")) {
			for (auto& kv : dialogOrphans)
				Text(kv.first);
			TreePop();
		}
	}

	
	if (TreeNode("Check Dialogs")) {
		for (auto& kv : dialFiles) {
			const char* file = kv.first.c_str();
			std::shared_ptr<DialogFile> d = kv.second;

			for (SheetLine& lineGroup : d->lines ) {
				auto& key = lineGroup.id;
				if (rd::String::startsWith(key, "!"))
					continue;
				if (rd::String::equalsI(key, "eof"))
					continue;
				for (auto& line : lineGroup.texts) {
					if (line.second.empty() ) {
						Text("No translation for "s + key);
					}
				}
			}
		}
		TreePop();
	}
	if (TreeNode("Check UI")) {
		for (auto& kv : uiFiles) {
			const char* key = kv.first.c_str();
			KeyText& k = kv.second;
			auto& l = k.kv[checkedLang];
			if (rd::String::startsWith(key,"!")) 
				continue;
			if (rd::String::equalsI(key,"eof")) 
				continue;
			if (l.text.empty()) {
				Text("No translation for "s + key);
			}
		}
		TreePop();
	}
}

void T::im() {
	using namespace ImGui;
	if (TreeNode("Dialog Files")) {
		if (TreeNode("Query")) {
			ImGui::InputText("Thread test query", dialQuery);
			if (!dialQuery.empty()) {
				for (auto& dfp : dialFiles) {
					for (auto& l : dfp.second->lines)
						if (rd::String::containsI(l.id, dialQuery)) {
							PushID(&l);
							if (Button(l.id))
								dialQuery = l.id;
							PopID();
						}
				}

				auto thr = getDialogs(dialQuery.c_str());
				if (!thr.empty()) {
					for (auto& l : thr) {
						auto line = l->getLine();
						Text(l->threadId); SameLine();
						Text(line);
					}
				}

			}
			TreePop();
		}

		for (auto& dfp : dialFiles) {
			PushID(dfp.first.c_str());
			LabelText("file", dfp.first.c_str());
			if (TreeNode("Content")) {
				for (auto& l : dfp.second->lines) {
					PushID(&l);
					if (ImGui::TreeNode((string("id:") + l.id.c_str()).c_str())) {
						for (auto& s : l.texts)
							ImGui::Text("%s -> %s", s.first.c_str(), s.second.c_str());
						TreePop();
					}
					if (IsItemHovered()) {
						string t;
						t += "index:"s + to_string(l.index)+"\n";
						t += "threadId:" + l.threadId + "\n";
						t += "speaker:" + l.charName + "\n";
						t += "trigger:" + std::to_string(l._trigger) +"\n";
						//t += "trigger:" + std::to_string(l.trigger) +"\n";
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
		if (TreeNode("Query")) {
			auto backupMiss = dumpMiss;
			dumpMiss = false;
			InputText("input", s_TempQuery);
			if (s_TempQuery.length()) {
				auto hasQuery = has(s_TempQuery.c_str());
				Value("Result for has():", hasQuery);
				auto getQuery = get(s_TempQuery.c_str());
				Text(getQuery.c_str());
				if (TreeNode("List matches")) {
					for (auto& t : uiFiles) {
						if (rd::String::containsI(t.first.c_str(), s_TempQuery)) {
							bool extended = false;

							if (!t.second.tags.empty())
								extended = true;

							if (extended) {
								if (TreeNode(t.first)) {
									t.second.im();
									TreePop();
								}
							}
							else {
								Text(t.first + " => " + t.second.getLine());
							}
						}
					}
					TreePop();
				}
			}
			TreePop();
			dumpMiss = backupMiss;
		}

		for (auto& ui : uiFiles) {
			PushID(ui.first.c_str());
			LabelText("id", ui.first.c_str());
			Indent();
			auto& line = ui.second;
			for (auto& s : ui.second.kv) {
				auto& e = s.second;
				ImGui::Text("%s -> %s", s.first.c_str(), e.text.c_str());
				if (IsItemHovered()) {
					std::string res;
					if (!line.tags.empty()) 
						res += "Nar: "+ line.tags.cpp_str()+" ";
					if (e.ast) 
						res += "Ast:" + Nar().stringify(e.ast) + " ";
					SetTooltip(res);
				}
			}
			Unindent();
			PopID();
		}
		TreePop();
	}

	
	imTranslationCheck();
}



void T::init(const string& newLang){
	curLang = newLang;

#ifndef PASTA_NX
	std::setlocale(LC_ALL, "en_US.UTF-8");
#endif

	if (!initialSystemLangFetched) {
		bool langAuthorized = false;
		char* lang = Pasta::TextMgr::getSingleton()->getLocalization();
		for (auto& lg : langs) {
			if (lg == lang) {
				curLang = lang;
				langAuthorized = true;
				break;
			}
		}
		if (!langAuthorized)
			curLang = DEFAULT_LANG;
		onLanguageChanged.trigger();
		initialSystemLangFetched = true;
		cout << "Main language set to " << curLang << endl;
	}
	isInit = true;
}

static rd::parse::CondContext dummyCtx;

bool T::evalCond(const char* trigger) {
	rd::parse::CondContext * ctx = triggerContext;
	if (!ctx) 
		ctx = &dummyCtx;
	rd::parse::CondParser cp;
	cp.lex = rd::parse::CondLexer::parse(trigger);
	cp.parse();
	return cp.eval(*ctx);
}

bool T::changeLang(const char * lang) {
	for (auto& availableLang : langs) {
		if (lang == availableLang) {
			curLang = lang;
			trace( "- Main language set to "s + curLang);
			onLanguageChanged.trigger();
			return true;
		}
	}
	traceError("Language "s + lang + " not found");
	return false;
}

void T::init(vector<string>& _langs){
	langs = _langs;
}

void T::destroy(){
	curLang="";
	uiFiles.clear();
	dialFiles.clear();
	isInit = false;
}

const Str& ri18n::T::getStr(const Str& key){
	return getStr(key.c_str());
}

const Str& T::getStr(const std::string& key) {
	return getStr(key.data());
}

const char* T::getCStr(const std::string& key) {
	return getCStr(key.c_str());
}

static const char* TEXT_ERR_TAG = "~";
const Str& T::getStr(const char* key) {
	ri18n::KeyText* file = rs::Std::get(uiFiles, key);
	if (!file) {
		s_TempKey = tolower(key);
		file = rs::Std::get(uiFiles, s_TempKey.data());
	}

	bool displayMiss = false;
	#ifdef PASTA_DEBUG
		displayMiss = false;
	#endif

	if (!displayMiss && !file) { // nothing found it is probably a missing key and not a missing translation
		dummyStr = key;
		return dummyStr;
	}

	if (!file) {
		if (dumpMiss) {
			trace(std::string() + "Unknown text key in uiFiles #" + key);
			uiOrphans[key]=true;
		}
		dummyStr.clear();
		dummyStr = string(TEXT_ERR_TAG) + key;
		return dummyStr;
	}
	auto e = rs::Std::get(file->kv, StrRef(curLang));
	if (!e) { // no corresponding content, probably a missing translation
		dummyStr.clear();
		dummyStr = string(TEXT_ERR_TAG)+key;
		uiOrphans[key] = true;
		return dummyStr;
	}
	if (ALLOW_PROCGEN_INJECTION) {
		if (e->gen)
			return procGenStr = e->gen->gen( "run" );
	}
	return e->text;
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
	auto e = rs::Std::get(file->kv, StrRef(curLang));
	if (!e)
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
	
	StrRef kStr(key);

	auto & id2Line = sheet->id2Line;
	if (id2Line.find(kStr) == id2Line.end()) {
		dialogOrphans[key] = true;
		return nullptr;
	}

	if (strcmp(threadId, "0") == 0) 
		return &(sheet->lines[id2Line[kStr]]);
	
	auto first = sheet->lines.begin() + id2Line[kStr];
	while( 
		(first != sheet->lines.end())
	&&	(first->threadId != threadId) )  {
		first++;
		if (first->id != key)
			return nullptr;
	}
	return &(*first);
}

SheetLine * T::getDialogLine(const char * key, const char * threadId, const char * sheet){
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
		auto& df = dialFiles[sheet];
		return getDialogLine(key, threadId, df);
	}
	return nullptr;
}

vector<SheetLine*> 
T::getThread(const char * key, const char * threadId, const char * sheet){

	StrRef kStr(key);

	if (threadId == nullptr) threadId = "0";
	if (sheet == nullptr) {
		for (auto dfp : dialFiles) {
			if (dfp.second->id2Line.find(kStr) != dfp.second->id2Line.end())
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

vector<SheetLine*> T::getThread(const char * key, const char * threadId, shared_ptr<DialogFile> sheet){
	if (threadId == nullptr) threadId = "0";
	vector<SheetLine*> res;

	auto lineStart = sheet->id2Line[key];

	if(!sheet->lines.size())
		return res;

	auto start = sheet->lines.begin() + lineStart;
	if (start == sheet->lines.end()) 
		return res;

	auto& next = start;
	const string & id = start->id;
	while(
		(next != sheet->lines.end()) 
	&&	(id == next->id) ) {
		if( next->threadId == threadId )
			res.push_back(&(*next));
		next++;
	}
	return res;
}

vector<SheetLine*> 
T::getDialogs(const char * key,  const char * sheet){
	if (sheet == nullptr) {
		for (auto& dfp : dialFiles) {
			if (dfp.second->id2Line.find(key) != dfp.second->id2Line.end())
				return getDialogs(key,  dfp.second);
		}
	}
	else {
		if (dialFiles.find(sheet) == dialFiles.end())
			return {};
		auto&df = dialFiles[sheet];
		return getDialogs(key,  df);
	}
	return {};
}

vector<SheetLine*>
T::getDialogs(const char * key, shared_ptr<DialogFile> sheet){
	vector<SheetLine*> res;
	auto start = sheet->lines.begin() + sheet->id2Line[key];
	auto next = start;
	const string & id = start->id;
	while (
		(next != sheet->lines.end())
		&& (id == next->id)) {
		res.push_back(&(*next));
		next++;
	}
	return res;
}

bool ri18n::T::hasDialog(const char* key) {
	auto threadId = "0";
	for (auto& dfp : dialFiles) {
		SheetLine* dl = getDialogLine(key, threadId, dfp.second);
		if (dl != nullptr)
			return true;
	}
	return false;
}

bool ri18n::T::hasDialog(const char* key, int threadId) {
	for (auto& dfp : dialFiles) {
		SheetLine* dl = getDialogLine(key, to_string(threadId).c_str(), dfp.second);
		if (dl != nullptr)
			return true;
	}
	return false;
}

void T::clearUITexts(){
	uiFiles.clear();
}


shared_ptr<DialogFile> T::loadThreadedFile(const char * filename ){
	int _line = 0;
	bool doProcess = true;

	unordered_map<string, int> lang_idx;
	unordered_map<int, string> idx_lang;

	int index_idx = -1;
	int key_idx = -1;
	int char_idx = -1;
	int thread_idx = -1;
	int cond_idx = -1;
	int trigger_idx = -1;
	int linePos = 0;
	int tags_idx = -1;

	string prevId = "";
	int curAutoThreadId = 0;
	string threadId = "";
	string id = "";

	shared_ptr< DialogFile> df = make_shared<DialogFile>();

	vector<SheetLine> allString;
	unordered_map<Str, int> id2Line;
	string empty="";

	std::function<void(vector<string>& line)> parse = [&](vector<string> & line) {
		if (!doProcess)return;

		if (_line == 0) {
			auto b = line.begin();
			auto e = line.end();
			int idx = 0;
			for (auto & l : line) {
				if (l == "id") key_idx = idx;
				else if (l == "index") index_idx = idx;
				else if (l == "key") key_idx = idx;
				else if (l == "char" ||  l == "character" || l == "Characters" || l == "speaker")
					char_idx = idx;
				else if (l == "thread") thread_idx = idx;
				else if (l == "cond") cond_idx = idx;
				else if (l == "trigger") trigger_idx = idx;
				else if (l == "tags") tags_idx = idx;
				else {
					bool found = false;
					for (auto & lg : langs) {
						if (lg == l) {
							lang_idx[lg] = idx;
							idx_lang[idx] = lg;
							found = true;
							//trace("Dialog:found lang "s + lg + " at column "s + to_string(idx));
							break;
						}
					}
					if (!found && l != string("")
						&& l != "images"
						&& l != "word"
						&& l != "count EN"
						&& l != "notes"
						&& l != "maxchars" )
						trace("Unrecognised column key ?s " + l );
				}

				idx++;
			}
		}
		else {
			bool isNewId = !rd::String::equalsI(prevId,id);

			string charName = empty;

			if (isNewId) 
				curAutoThreadId = 0;

			if (char_idx == -1) 
				trace( "unable to find character column" );
			else 
				charName = line[char_idx];

			//create key and update thread infos
			const string& idx = line[index_idx];
			
			const string& key = line[key_idx];
			if (key.empty()) { // no key we skip!
				_line++;
				return;
			}

			if (charName.empty()) { // no char we skip!
				_line++;
				return;
			}
			
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
				std::move(texts)
			);
			t.index = atoi(line[index_idx].c_str());
			t.tags = line[tags_idx];
			if ((trigger_idx >= 0) && line[trigger_idx].length())
				t.setTrigger(line[trigger_idx].c_str());
			t.linePos = linePos;
			allString.push_back(t);
			if (isNewId) {
				id2Line[id] = allString.size() - 2;
				linePos = 0;
			}
		}
		_line++;
	};
	fmt::Tsv::parseAndAssign(filename, parse);
	std::swap(df->lines, allString);
	std::swap(df->id2Line, id2Line);
	auto & cdf = dialFiles[filename] = df;

	compileDialogFile(cdf);
	traceWarning(""s + filename + " nb lines read "s + to_string(_line));
	return df;
}

static int s_tagsIdx = -1;
void T::loadUIFile(const char * filename){
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
					&& l != "tags"
					&& l != "maxchars"
					) {
					
					//trace( "Unrecognised lang ? "s+ l );
				}
				if (l == "tags")
					s_tagsIdx = idx;
				idx++;
			}
		}
		else if(tmp_do_process) {
			for (int i = 0; i < line.size(); ++i) {
				if (tmp_idx_lang.find(i) != tmp_idx_lang.end()) {
					s_TempKey = tolower(line[0]);

					uiFiles[line[0]].kv[tmp_idx_lang[i]] = { line[i],  0 };

					auto& v = uiFiles[line[0]];
					if (s_tagsIdx >= 0)
						v.tags = line[s_tagsIdx];

					if (s_TempKey != line[0])
						uiFiles[Str(s_TempKey)].kv[tmp_idx_lang[i]] = { line[i],  0 };
				}
			}
			
			if (line[0] == "FIN" || line[0] == "EOF" || line[0] == "END") {
				postProcessAllUITexts();
				tmp_do_process = false;
				s_tagsIdx = -1;
			}
		}
	};

	fmt::Tsv::watchParseAndAssign(filename, parse );
}

const char * utf8EEszet = u8"ẞ";
const char * eszet = "ß";
const char * eszetMaj = "SS";



const char* utf8iter(const char* pos, int& code) {
	if (!pos) return 0;
	utf8codepoint(pos, &code);
	if (!*pos)
		return 0;
	return pos + utf8codepointsize(code);
}

void getUtf8fromC32(int code, char* utf8) {
	int sz = utf8codepointsize(code);
	utf8catcodepoint(utf8, code, 4);
}

std::string T::tolower(const char* str) {
	string res; res.reserve(utf8len(str));
	const char* start = str;
	int cp = 0;
	char cpc[5] = {};
	while (start) {
		memset(cpc, 0, 4);
		start = utf8iter(start, cp);
		getUtf8fromC32( utf8lwrcodepoint(cp), cpc);
		res += cpc;
	}
	for (auto& kv : s_up) {
		size_t pos = res.find(kv.second);
		if (pos != std::string::npos)
			res.replace(pos, kv.second.length(), kv.first);
	}
	return res;
}

string T::toupper(const char* str) {
	string res; res.reserve(utf8len(str));
	const char* start = str;
	int cp = 0;
	char cpc[5] = {};
	while (start) {
		memset(cpc,0,4);
		start = utf8iter(start, cp);
		getUtf8fromC32(utf8uprcodepoint(cp),cpc);
		res += cpc;
	}
	for (auto& kv : s_up) {
		size_t pos = res.find(kv.first);
		if (pos != std::string::npos) 
			res.replace(pos, kv.first.length(), kv.second);
	}
	return res;
}

string T::toupper(const std::string& str) {
	return toupper(str.c_str());
};

Str T::toupper(const Str& str) {
	return Str(toupper(str.cpp_str()));
};


Str T::tolower(Str& str){
	Str res; 
	res = tolower(str.c_str());
	return res;
}

std::string T::tolower(const std::string& str) {
	return tolower(str.c_str());
}

shared_ptr<DialogFile> T::getDialogSheet(const char * name){
	if (dialFiles.find(name) == dialFiles.end())
		return nullptr;
	return dialFiles[name];
}

void T::testAST(const char * filename){
	std::vector<shared_ptr<DialogFile>> dials;

	for (const auto & lg : langs) {
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
			//SAMPLE ASt comparison tool
			/*
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
			*/
		}
		nLine++;
	}
}



void T::compileDialogFile(shared_ptr<DialogFile> f){
	Nar n;
	int nbCompiles = 0;
	//do not make compilation for now as we don't need the results in the game right now
	for (auto & lines : f->lines) {
		//Str* res = rs::Std::get(lines.texts, StrRef(curLang));
		//lines.ast = n.make( );
		if ( rd::String::hasTag(lines.tags,"script")) {
			lines.asts["en"] = n.make( lines.texts["en"].c_str() );
			nbCompiles++;
		}
	}

	for (auto& lines : f->lines) 
		lines.postProcess();
	
	trace("compiled "s + to_string(nbCompiles) + " scripts");
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
		changeLang(lang.c_str());

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
	if (!str || !*str) return s_emptyCppStr;
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

Str	ri18n::T::toCapitalized(const char* l) {
	return Str(rd::String::toCapitalized(l));
}

void ri18n::T::capitalize(std::string& label){
	if (label.empty())
		return;
	auto vec = rd::String::utf8ToC32(label);
	std::vector<int> res;
	res.reserve(vec.size());
	res.push_back(std::toupper(vec[0]));
	for (int i = 1; i < vec.size();++i) {
		if (vec[i - 1] == ' ') 
			res.push_back(std::toupper(vec[i]));
		else
			res.push_back(std::tolower(vec[i]));
	}
	res.push_back(0);
	label = rd::String::c32toUtf8(res.data());
}

void ri18n::T::firstCapitalize(Str& label) {
	std::string l = label.cpp_str();
	ri18n::T::firstCapitalize(l);
	label = l;
}

std::string ri18n::T::format(const char* str, const char* p0, const char* p1, const char* p2) {
	std::string s = str;
	if (rd::String::contains(str, "$0")) {
		auto ss = rd::String::replace(s, "$0", p0);
		s = ss;
	}
	if (rd::String::contains(str, "$1")) {
		auto ss = rd::String::replace(s, "$1", p1);
		s = ss;
	}
	if (rd::String::contains(str, "$AP")) {
		auto ss = rd::String::replace(s, "$AP", p1);
		s = ss;
	}
	return s;
}

std::string ri18n::T::format(const char* str, const char* p0, const char* p1) {
	std::string s = str;
	if (rd::String::contains(str, "$0")){
		auto ss = rd::String::replace(s, "$0", p0);
		s = ss;
	}
	if (rd::String::contains(str, "$1")){
		auto ss = rd::String::replace(s, "$1", p1);
		s = ss;
	}
	return s;
}

std::string ri18n::T::format(const char* str, std::initializer_list<std::pair<const char*, const char*>> mp){
	std::string s = str;
	for (auto& p : mp) {
		if ( !rd::String::contains(str, p.first)) 
			continue;
		std::string ss = rd::String::replace(s, p.first, p.second);
		s = ss;
	}
	return s;
}

std::string ri18n::T::format(const char* str, std::unordered_map<const char*,const char *> &mp){
	std::string s = str;
	for(auto & p : mp){
		if (!rd::String::contains(str, p.first))
			continue;
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

const char* TD(const char* id) {
	auto& l = ri18n::T::getDialogs(id);
	if (l.empty())
		return id;
	return l[0]->texts[ri18n::T::getLang()].c_str();
}

const char* TU(const char* id) {
	tuStr = ri18n::T::toupper(StrRef(TT(id)));
	return tuStr.c_str();
}

Str TFirstCap(const char* id) {
	std::string label = ri18n::T::get(id);
	ri18n::T::firstCapitalize(label);
	return Str(label);
};

Str TFst(const char* id) {
	return TFirstCap(id);
}

Str TFst(const std::string& id) {
	return TFirstCap(id.c_str());
}

std::string TF(const char* id, const char* p0) {
	bool plural = !(rd::String::equals(p0, "0") || rd::String::equals(p0, "1"));
	return ri18n::T::format(TT(id), { std::make_pair("$0",p0), std::make_pair("$s", plural?"s":"") });
}

std::string TF(const char* id, const char* p0, const char* p1) {
	return ri18n::T::format(TT(id), { std::make_pair("$0",p0),std::make_pair("$1",p1) });
}

std::string TF(const char* id, const char* p0, const char* p1, const char* p2)
{
	return ri18n::T::format(TT(id), { std::make_pair("$0",p0), std::make_pair("$1",p1),std::make_pair("$2",p2) });
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

std::string TF(const char* id, int p0, int p1, int p2){
	return TF(id, std::to_string(p0).c_str(), std::to_string(p1).c_str(), std::to_string(p2).c_str());
}

bool ri18n::SheetLine::evalTrigger(){

	return false;
}

ri18n::SheetLine::SheetLine(const string& id, const string& threadId, const string& _charName, unordered_map<Str, Str> _texts, bool isThreadEnd) {
	this->id = id;
	this->threadId = threadId;
	this->charName = _charName;
	this->texts = _texts;

	if (!rd::String::isLower(charName)) 
		charName = rd::String::toLower(charName);
}

void ri18n::T::postProcessAllUITexts() {
	auto t0 = rs::Timer::stamp();
	for (auto& lines : uiFiles) {
		auto& v = lines.second;
		v.postProcess();
	}
	auto t1 = rs::Timer::stamp();
	trace("time to post process all ui texts : " + std::to_string(t1 - t0) + "s");
}

void ri18n::KeyText::postProcess(){
	Nar n;

	//we could optimize and not roll over all the file by using a range thing
	//only for preprocess in case of bad performances of ast fetching
	if (rd::String::hasTag(tags, "ast") )
		for (auto& lgs : kv) {
			if (lgs.second.ast) delete lgs.second.ast;
			lgs.second.ast = n.make(lgs.second.text.c_str());
		}

	if (rd::String::hasTag(tags, "procGen"))
		for (auto& lgs : kv) {
			if (lgs.second.gen) delete lgs.second.gen;
			lgs.second.gen = ri18n::RandText::parse(lgs.second.text.c_str());
		}

	for (auto& lgs : kv) {
		for (auto& rp : s_rps) {
			auto& line = lgs.second;
			if (rd::String::contains(line.text.c_str(), rp.first)) {
				line.text = rd::String::replace(line.text, rp.first, rp.second);
			}
		}
	}

	for (auto t : T::processors) {
		for (auto& lgs : kv) {
			auto& line = lgs.second.text;
			if (t->process(line)) {
				int here = 0;
			}
		}
	}
}

const Str& ri18n::KeyText::getLine() const{
	auto ref = StrRef(T::getLang().c_str());
	auto p = rs::Std::get(kv, ref);
	if (!p)
		return s_emptyStr;
	return p->text;
}

void ri18n::KeyText::im(){
	using namespace ImGui;
	LabelText("tags", tags.c_str());
	for (auto& lgs : kv) {
		Text(lgs.first.c_str());
		lgs.second.im();
	}
}

const Str& ri18n::SheetLine::getLine() const{
	auto ref = StrRef(T::getLang().c_str());
	auto p = rs::Std::get(texts, ref);
	if (!p) 
		return s_emptyStr;
	return *p;
}

Str& ri18n::SheetLine::getLine() {
	auto ref = StrRef(T::getLang().c_str());
	auto p = rs::Std::get(texts, ref);
	if (!p)
		return s_emptyStr;
	return *p;
}

void ri18n::SheetLine::setTrigger(const char* trg){
	if (!trg) return;
	if (!*trg) return;
	_trigger = trg;
	trigger = rd::parse::CondParser::parse(trg);
}

void ri18n::SheetLine::postProcess(){
	if( rd::String::containsI(texts["fr"], "Oh. Il"))
		int here = 0;
	for (auto& lgs : texts) {
		for (auto& rp : s_rps) {
			auto& line = lgs.second;
			if (rd::String::contains(line.c_str(), rp.first)) 
				line = rd::String::replace(line, rp.first, rp.second);
		}
	}
	int here = 0;
}

void ri18n::UISheetLine::im(){
	using namespace ImGui;
	LabelText("value", text.c_str());
	if (ast)
		ast->im();
	if (gen)
		gen->im();
}
