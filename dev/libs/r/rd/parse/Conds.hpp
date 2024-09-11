#pragma once
namespace rd {
	namespace parse {
		enum class TokenKind : r::u8 {
			None,
			ParOpen,
			ParClose,
			QuestionMark,
			ExclamationMark,
			Comma,
			Colon,
			Identifier,
			String,
			Float,
			Int,
			Str,

			LogicalOr,
			LogicalAnd,

			BitwiseOr,
			BitwiseAnd,

			Gt,
			Ge,
			Lt,
			Le,
			Eq,
			NEq,

			True,
			False,

			OpPlus,
			OpMinus,
			OpMul,
			OpDiv,
		};

		union RawVal {
			int		ival;
			double	dval;
		};

		struct Token {
			TokenKind	kind = TokenKind::None;
			RawVal		data{};
			std::string	sdata;

			void im();
		};

		struct CondLexer {
			Str							debug;
			eastl::vector<Token>		tokens;
			void						im();
			static CondLexer			parse(const char* str);
			static eastl::vector<Token>	lex(const char* str);

		};

		enum class CondNodeKind : r::u8 {
			None = 0,
			Expr = 1 << 0,//
			ControlUnary = 1 << 1,
			ControlBinary = 1 << 2,
			ControlTernary = 1 << 3,
			Funcall = 1 << 4,
		};
		R_ENUM_BITS_OPERATORS(CondNodeKind)

			struct CondNode {
			CondNodeKind				kind = CondNodeKind::None;
			Token* t = 0;
			CondNode* pred = 0;
			CondNode* l = 0;
			CondNode* r = 0;
			eastl::vector<CondNode*>	args;

			CondNode(Token* _t) { t = _t; kind = CondNodeKind::Expr; };
			void						im();
		};

		struct CondContext {

			//simple api
			std::function<std::any(const char* fun, eastl::vector<std::any>& args)>
				onFuncall;

			//complex api for debug and berzerkers
			std::function<std::any(CondNode& node)>		onFuncallAst;
			std::function<std::any(const char* id)>	onSingleIdentifier;
			CondContext();
		};

		class CondParser {
		public:
			CondLexer					lex;
			CondNode* root = 0;
			r::qbool					success = Undetermined;
			rd::parse::CondNode*	parseAndResume(CondLexer& lex, int pos, int& outputNext);
			CondNode*				parse(CondLexer& lex, int pos, int& outputNext);
			static CondParser*		parse(const char* str);
			void						parse();
			void						im();

			bool						eval(CondContext& ctx);
			std::any					evalOp(CondContext& ctx);
		protected:
			std::any					eval(CondContext& ctx, CondNode& cur);
		};
	}
}