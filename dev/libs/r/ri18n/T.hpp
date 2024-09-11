#pragma once

#include "rd/Sig.hpp"
#include "Lex.hpp"
#include "rd/BitArray.hpp"

namespace rd {
	namespace parse {
		class CondParser;
		struct CondContext;
	}
}
namespace ri18n {
	struct	AstNode;
	class	RandText;

	struct UISheetLine {
		Str			text;
		AstNode*	ast = 0;
		RandText*	gen = 0;

		void		im();
	};

	struct KeyText {
		//maps langs to words
		Str										tags;
		Str										_trigger;
		rd::parse::CondParser*					trigger{};
		std::unordered_map<Str, UISheetLine>	kv;// ( lang_id * line )

		void									postProcess();
		const Str&								getLine() const;
		void									im();
	};

	struct SheetLine {
		int									index=0;
		int									linePos=0;
		std::string							id;
		std::string							threadId;
		std::string							charName;
		Str									tags;

		//should use flat maps
		std::unordered_map<Str, Str>		texts;//lang 2 texts
		std::unordered_map<Str, AstNode*>	asts;//lang 2 asts

		Str									_trigger;
		rd::parse::CondParser*				trigger{};

		bool								evalTrigger();

		SheetLine(
			const std::string & id,
			const std::string & threadId, 
			const std::string & charName, 
			std::unordered_map<Str, Str> _texts,
			bool isThreadEnd = false);
		const Str&							getLine() const;
		Str&								getLine();

		void								setTrigger(const char* trg);
		void								postProcess();

	};
	
	struct DialogFile {
		std::vector<SheetLine>						lines;
		std::unordered_map<Str, int>				id2Line;//returns the position of the first id in list
	};

	typedef std::function<Str(const Str&)> TextReplacer;

	struct ITextprocessor {
		virtual bool process(Str& inOut) = 0;
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

		static std::string			DEFAULT_LANG;
		static std::string			dummyStdStr;
		static Str					dummyStr;
		static Str					emptyStr;
		static Str					procGenStr;
		static bool					dumpMiss;

		//allows the engine to inject procgen texts in regular functions
		static bool					ALLOW_PROCGEN_INJECTION;

		static void					init();
		static void					init(const std::string & newLang);
		static void					init(std::vector<std::string>&langs);

		static bool					imOpened;
		static void					im();
		static void					imWindowed();
		static void					imTranslationCheck();

		static void					destroy();

		//key will fallback to lowercase if need be
		//if string doesn't exist and empty string will be returned, test for has to prevent this
		static const Str&			getStr(const std::string& key);
		static const Str&			getStr(const char* key);
		static const Str&			getStr(const Str& key);

		static const char*			getCStr(const char* key);
		static const char*			getCStr(const std::string& key);
		static inline const char*	getCStr(const Str& key) { return getCStr(key.c_str()); };

		static std::string			get(const char* key);
		static std::string			get(const std::string& key);
		static inline std::string	get(const Str& key) { return get(key.c_str()); };


		static bool					has(const char* key);
		static inline bool			has(const Str& key) { return has(key.c_str()); };
		static inline bool			has(const std::string & key) { return has(key.c_str()); };

		static Str					getStrU(const char* key);
		static std::string			getU(const char* key);

		static bool					changeLang(const char* lang);
		static bool					evalCond( const char * trigger);

		static SheetLine*	
			getDialogLine(const char * key, const char * threadId = nullptr, const char * sheet = nullptr);

		static SheetLine*
			getDialogLine(const char * key, const char * threadId, std::shared_ptr<DialogFile> sheet);

		static std::vector<SheetLine*>
			getThread(const char * key, const char * threadId = nullptr, const char * sheet = nullptr);

		static std::vector<SheetLine*>
			getThread(const char * key, const char * threadId, std::shared_ptr<DialogFile> sheet);

		static std::vector<SheetLine*>
			getDialogs(const char * key, const char * sheet = nullptr);

		static std::vector<SheetLine*>
			getDialogs(const char * key, std::shared_ptr<DialogFile> sheet);

		static bool
			hasDialog(const char* key);

		static bool
			hasDialog( const char* key, int threadId );

		static void						clearUITexts();
		static void						loadUIFile(const char * filename);

		static std::shared_ptr<DialogFile>	loadThreadedFile(const char * filename);

		//Perfect to upper case func, may make a small alloc
		static std::string				toupper(const char* str);
		static std::string				toupper(const std::string & str);
		static Str						toupper(const Str & str);

		static std::string				tolower(const char* str);
		static std::string				tolower(const std::string & str);
		static Str						tolower(Str& str);

		//static std::pair<string, rd::Vars> parseEnum(const char* src);

		static std::shared_ptr<DialogFile>	getDialogSheet(const char * filename);

		static void						testAST(const char * filename);
		static void						compileDialogFile(std::shared_ptr<DialogFile>);
		
		//https://fr.wikipedia.org/wiki/Liste_des_codes_ISO_639-1 or nearly it
		static const std::string&			getLang() { return curLang; };
		static eastl::vector<std::string>	listLangs();

		static bool						emitCharsets(std::vector<std::string> langs, const char* out);

		static std::vector<char>		getSoftPunctuation();
		static std::vector<char>		getHardPunctuation();
		
		static void						capitalize(std::string& label);
		static Str						toCapitalized(const char * l);
		static void						firstCapitalize(std::string& label);
		static void						firstCapitalize(Str& label);

		static std::string				firstCapitalize(const char * str);
		static std::string				gameplayNumber(float f);

		static std::string 				format(const char* str, std::initializer_list<std::pair<const char*, const char*>> mp);
		static std::string 				format(const char* str, std::unordered_map<const char*, const char*>& mp);
		static std::string 				format(const char* str, std::unordered_map<std::string,std::string> &mp);
		static std::string 				format(const char* str, std::initializer_list<std::pair<std::string,std::string>> mp);
		static std::string 				format(std::string& str, std::initializer_list<std::pair<std::string, std::string>> mp) {
			return format(str.c_str(), std::move(mp));
		};

		static std::string 				format(const char* str, const char* p0, const char* p1);
		static std::string 				format(const char* str, const char* p0, const char* p1, const char* p2);
		static std::string 				format(const char* str, int p0, int p1) {
			return format(str, std::to_string(p0).c_str(), std::to_string(p1).c_str());
		};

		static std::function<void(int,rd::Font*)>	onUnknownFontCharacterEncountered;
		static void						postProcessAllUITexts();

		static std::vector<ITextprocessor*> processors;

	protected:
		static bool													isInit;
		static bool													initialSystemLangFetched;

		static std::string											curLang;

		//for each line, store its per lang counterpart 
		static std::unordered_map<Str, KeyText>						uiFiles;
		static std::unordered_map<Str, std::shared_ptr<DialogFile>>	dialFiles;
		static rd::Sig												onLanguageChanged;

		static bool													tmp_do_process;
		static std::unordered_map<std::string, int>					tmp_lang_idx;
		static std::unordered_map<int, std::string>					tmp_idx_lang;

		static std::unordered_map<Str,bool>							uiOrphans;
		static std::unordered_map<Str,bool>							dialogOrphans;

	//test_tools
	public:
		static rd::parse::CondContext*	triggerContext;
		static auto&					getUIFiles() { return uiFiles;};
		static auto&					getDialFiles() { return dialFiles;};

		static bool						debugWarningCompareAst(AstNode*ref, AstNode*tgt);
		static bool						debugErrorCompareAst(AstNode*ref, AstNode*tgt);
	};
}

inline const char* TT(const Str& id) {
	return ri18n::T::getCStr(id);
};

inline const char* TT(const char* id) {
	return ri18n::T::getCStr(id);
};

extern const char* TD(const char* id);
extern const char* TU(const char* id);

extern Str TFirstCap(const char* id);

extern Str TFst(const char* id);
extern Str TFst(const std::string&id);

extern std::string	TF(const char* id, const char* p0);
extern std::string	TF(const char* id, const char* p0, const char* p1);
extern std::string	TF(const char* id, const char* p0, const char* p1, const char* p2);

extern std::string	TF(const char* id, const std::string &p0);
extern std::string	TF(const char* id, const std::string& p0, const std::string & p1);
extern std::string	TF(const char* id, int p0);
extern std::string	TF(const char* id, int p0, int p1);
extern std::string	TF(const char* id, int p0, int p1, int p2);

extern const char*	utf8iter(const char* pos, int& code);

