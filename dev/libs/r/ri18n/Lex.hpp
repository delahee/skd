
#pragma once

namespace ri18n {
	enum LexemType {
		Char,//c:Int
		Pipe,
		Literal,//str
		AccOpen,
		AccCondOpen,
		AccClose,
		TagOpen,//str
		TagClose,//str
		TagSelfClosed,//str
		BrackOpen,
		BrackClose,
		BrackCondOpen,//[?...] signals conditionnal event
		BrackImportantOpen,//[!...] signals system event
		Star,
		DoubleStar,
		DoubleSemiColon,
		BrackPosOpen,

		LexemTypeInvalid,
	};

	struct Lexem {
		LexemType	type	= LexemTypeInvalid;
		int			cc		= 0;
		std::string strData;

		Lexem(LexemType type) {
			this->type = type;
		};

		Lexem(LexemType type,const std::string & d ) {
			this->type = type;
			strData = d;
		};

		Lexem(LexemType type, int cc) {
			this->type = type;
			this->cc = cc;
		};

	};

	class Lex {
	public:
		const bool			DEBUG = false;
		const char* cur = 0;

		Lex();

		bool	isSpecialChar(int cc);
		bool	isSentenceChar(int cc);

		std::vector<Lexem>	parse(const std::string& str);
		std::vector<Lexem>	parse(const char* str);

		static std::string	fromCharCode(int code);
		static std::string	lexemToString(const Lexem & l);
		static std::string	lexemsToString(const std::vector<Lexem> & l);
		static std::string	dumpLexems(const std::vector<Lexem> & l);
	protected:
		void _parse(const std::vector<int>& str, int pos, std::vector<Lexem>&res);
	};
}