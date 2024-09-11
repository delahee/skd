#pragma once

namespace rd {
	class Style;
}

namespace rui {
	namespace xv {

		struct Selector {
			Str					id;
			eastl::vector<Str>	classes;

			void				im();
			void				setClasses(const char* cls);
			bool				matchesClasses(const Selector& sel) const;
			bool				matches(const Selector& sel) const;
		};

		struct StyleSheetEntry {
			Selector			sel;
			rd::Style*			st = 0;

								StyleSheetEntry();
								StyleSheetEntry(const StyleSheetEntry& o);
			StyleSheetEntry&	operator=(const StyleSheetEntry& o);
			void				im();
			
		};

		enum TokType : r::u8 {
			Ident,
			String,
			Int,
			Float,
			Colon,
			Sharp,
			POpen,
			PClose,
			Exclam,
			Comma,
			Percent,
			Semicolon,
			BrOpen,
			BrClose,
			Dot,
			Spaces,
			//Dash,
			Equal,
			Slash,
			AntiSlash,
			Star,
			Eof,
		};

		struct Tok {
			TokType		id			= Eof;
			char*		dataHead	= 0;
			char*		dataEnd		= 0;

						Tok(TokType t, char* head = 0) { id = t; dataHead = head; };
			void		im();
			Str			extractLiteral();
		};

		class StyleSheetLexer {
		public:
			eastl::vector<Tok>	buf;
			const char*			head = 0;
			Str					errorLog;
								
								StyleSheetLexer() {};
								StyleSheetLexer(char* str);
			void				lex(char* str);
			void				im();
		};

		class StyleSheetParser {
		public:
		};

		class StyleSheet : public rd::Agent {
		public:
			eastl::vector<StyleSheetEntry>	styleRules;
		protected:
			StyleSheetLexer					lexer;
		public:

					StyleSheet();
			virtual ~StyleSheet();

			bool	parse(char * head);
			bool	im()override;
			void	append(const StyleSheet& se);
			eastl::vector<StyleSheetEntry*> infer(const Selector& sel);
		protected:
			bool	parseSymbols(StyleSheetLexer& lex);
			bool	parseSymbols(StyleSheetLexer&lex, int idx);
			int		parseSelector(StyleSheetLexer&lex, int idx, Selector&sel);
			int		parseBlock(StyleSheetLexer& lex, int idx, rd::Style& st);

			void	addStyle(StyleSheetEntry & se);
			
		};
	}
}

namespace std {
	std::string to_string(const rui::xv::Selector& sel);
}