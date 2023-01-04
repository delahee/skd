#pragma once

#include <vector>
#include <unordered_map>
#include <string>

#include "rd/Sig.hpp"
#include "Lex.hpp"
#include "Nar.hpp"
#include "rd/BitArray.hpp"

using namespace std;

namespace ri18n {

	struct KeyText {
		//maps langs to words
		unordered_map<Str, Str> kv;
	};

	struct SheetLine {
		string					id;
		string					threadId;
		string					charName;
		unordered_map<Str, Str> texts;//lang 2 texts
		string					cond;

		AstNode*				ast = nullptr;

		SheetLine(
			const string &id,
			const string & threadId, 
			const string & charName, 
			unordered_map<Str, Str> _texts,
			const string & cond,
			bool isThreadEnd = false){
			this->id = id;
			this->threadId = threadId;
			this->charName = charName;
			this->texts = _texts;
			this->cond = cond;
		};
	};

	
	struct DialogFile {
		vector<SheetLine> lines;
		unordered_map<string, int> id2Line;//returns the position of the first id in list
	};

	/// <summary>
	/// beware class is not thread safe because of internal buffers and std stuff
	/// stores keys in lowercase as well as normal case to ease fetching
	/// </summary>
	class T {
	public:
		constexpr static const char* fr = "fr";
		constexpr static const char* en = "en";
		constexpr static const char* zn = "zn";
		constexpr static const char* pt = "pt";
		constexpr static const char* es = "es";
		constexpr static const char* it = "it";
		constexpr static const char* ru = "ru";
		constexpr static const char* de = "de";
		constexpr static const char* cn = "cn";
		constexpr static const char* jp = "jp";
		static const char*			euroChars; 


		static std::string			dummyStdStr;
		static Str					dummyStr;
		static bool					dumpMiss;

		static void					init();
		static void					init(const string & defaultLang);
		static void					init(vector<string>&langs);

		static bool					imOpened;
		static void					im();
		static void					imWindowed();

		static void					destroy();

		//key will fallback to lowercase if need be
		//if string doesn't exist and empty string will be returned, test for has to prevent this
		static const Str&			getStr(const string& key);
		static const Str&			getStr(const char* key);
		static const Str&			getStr(const Str& key);

		static const char*			getCStr(const char* key);
		static const char*			getCStr(const string& key);
		static inline const char*	getCStr(const Str& key) { return getCStr(key.c_str()); };

		static std::string			get(const char* key);
		static std::string			get(const std::string& key);
		static inline std::string	get(const Str& key) { return get(key.c_str()); };


		static bool					has(const char* key);
		static inline bool			has(const Str& key) { return has(key.c_str()); };
		static inline bool			has(const std::string & key) { return has(key.c_str()); };

		static Str					getStrU(const char* key);
		static std::string			getU(const char* key);

		static void					changeLang(const string& lang);

		static SheetLine*	
			getDialogLine(const char * key, const char * threadId = nullptr, const char * sheet = nullptr);

		static SheetLine*
			getDialogLine(const char * key, const char * threadId, shared_ptr<DialogFile> sheet);

		static vector<SheetLine*> 
			getThread(const char * key, const char * threadId = nullptr, const char * sheet = nullptr);

		static vector<SheetLine*>
			getThread(const char * key, const char * threadId, shared_ptr<DialogFile> sheet);

		static vector<SheetLine*>
			getDialogs(const char * key, const char * threadId = nullptr, const char * sheet = nullptr);

		static vector<SheetLine*>
			getDialogs(const char * key, const char * threadId, shared_ptr<DialogFile> sheet);

		static void						clearUiTexts();
		static void						loadUiFile(const char * filename);

		static shared_ptr<DialogFile>	loadThreadedFile(const char * filename);

		//Perfect to upper case func, may make a small alloc
		static string					toupper(const string & str);
		static string					toupper(const char* str);
		static Str						toupper(const Str & str);

		static string					tolower(const string & str);
		static string					tolower(const char* str);
		static Str						tolower(Str& str);

		//static std::pair<string, rd::Vars> parseEnum(const char* src);

		static shared_ptr<DialogFile>	getDialogSheet(const char * filename);

		static void						testAST(const char * filename);
		static void						compileDialogFile(shared_ptr<DialogFile>);

		static const string&			getLang() { return curLang; };

		static bool						emitCharsets(vector<string> langs, const char* out);

		static vector<char>				getSoftPunctuation();
		static vector<char>				getHardPunctuation();

		static void						firstCapitalize(std::string& label);
		static std::string				firstCapitalize(const char * str);
		static std::string				gameplayNumber(float f);

		static std::string 				format(const char* str, std::initializer_list<std::pair<const char*, const char*>> mp);
		static std::string 				format(const char* str, std::unordered_map<const char*, const char*>& mp);
		static std::string 				format(const char* str, std::unordered_map<std::string,std::string> &mp);
		static std::string 				format(const char* str, std::initializer_list<std::pair<std::string,std::string>> mp);
		static std::string 				format(std::string & str, std::initializer_list<std::pair<std::string,std::string>> mp){
			return format(str.c_str(), std::move(mp));
		}

		static std::function<void(int,rd::Font*)>	onUnknownFontCharacterEncountered;

	protected:
		static string											curLang;
		static unordered_map<Str, KeyText>						uiFiles;
		static unordered_map<Str, shared_ptr<DialogFile>>		dialFiles;
		static rd::Sig											onLanguageChanged;

		static unordered_map<string, int>						tmp_lang_idx;
		static unordered_map<int, string>						tmp_idx_lang;
		static bool												tmp_do_process;

	//test_tools
	public:
		static bool						debugWarningCompareAst(AstNode*ref, AstNode*tgt);
		static bool						debugErrorCompareAst(AstNode*ref, AstNode*tgt);
	};
}

inline const char* TT(const char* id) {
	return ri18n::T::getCStr(id);
};

inline Str TU(const char* id) {
	return ri18n::T::getStrU(id);
};

inline Str TFirstCap(const char* id) {
	std::string label = ri18n::T::getU(id);
	ri18n::T::firstCapitalize(label);
	return Str(label);
};

inline Str TFst(const char* id) {
	return TFirstCap(id);
}

extern std::string	TF(const char* id, const char* p0);
extern std::string	TF(const char* id, const char* p0, const char* p1);

extern std::string	TF(const char* id, const std::string &p0);
extern std::string	TF(const char* id, const std::string& p0, const std::string & p1);
extern std::string	TF(const char* id, int p0);
extern std::string	TF(const char* id, int p0, int p1);
