#include "stdafx.h"

#include "Conds.hpp"

using namespace rd;
using namespace rd::parse;
using namespace std::string_literals;

void Token::im() {
	using namespace ImGui;
	typedef rd::parse::TokenKind Tk;
	auto id = kind;
#define DO( Ident )  case Ident: Text( # Ident ); break;
#define DO_S( Ident )  case Ident: Text( # Ident ); SameLine(); Text( sdata); break;
#define DO_F( Ident )  case Ident: Text( # Ident ); SameLine(); Text( "%f", data.dval); break;
#define DO_I( Ident )  case Ident: Text( # Ident ); SameLine(); Text( "%d", data.ival); break;
	switch (id) {
		DO(Tk::None)
		DO(Tk::ParOpen)
		DO(Tk::ParClose)
		DO(Tk::QuestionMark)
		DO(Tk::ExclamationMark)
		DO(Tk::Comma)
		DO(Tk::Colon)
		DO_S(Tk::String)
		DO_S(Tk::Identifier)
		DO_F(Tk::Float)
		DO_I(Tk::Int)
		DO(Tk::Str)
		DO(Tk::LogicalOr)
		DO(Tk::LogicalAnd)
		DO(Tk::BitwiseOr)
		DO(Tk::BitwiseAnd)
		DO(Tk::Gt)
		DO(Tk::Ge)
		DO(Tk::Lt)
		DO(Tk::Le)
		DO(Tk::Eq)
		DO(Tk::NEq)
		DO(Tk::True)
		DO(Tk::False)
		DO(Tk::OpPlus)
		DO(Tk::OpMinus)
		DO(Tk::OpMul)
		DO(Tk::OpDiv)
	default:
		break;
	}
#undef DO

}

void CondLexer::im() {
	using namespace ImGui;
	for (auto& t : tokens) 
		t.im();
}

CondLexer CondLexer::parse(const char* str) {
	CondLexer res;
	res.debug = str;
	res.tokens = lex(str);
	return res;
}

eastl::vector<Token> rd::parse::CondLexer::lex(const char* str){
	if (!str) 
		return {};
	if (!*str)
		return {};

	eastl::vector<Token> res;
	auto Tok = [](TokenKind k) {
		Token d;
		d.kind = k;
		return d;
	};
	auto TokDataS = [](TokenKind k,const char * val) {
		Token d;
		d.kind = k;
		d.sdata = val;
		return d;
	};
	auto TokDataI = [](TokenKind k, int val) {
		Token d;
		d.kind = k;
		d.data.ival = val;
		return d;

	};
	auto TokDataD = [](TokenKind k, double val) {
		Token d;
		d.kind = k;
		d.data.dval = val;
		return d;
	};

	auto hasKeyword = [](const char* str, const char* kw) {
		if (!rd::String::startsWithI(str, kw))
			return false;
		const char* after = str + strlen(kw);
		if (!*after)
			return true;
		if (*after == '(')
			return true;
		if (*after == ' ')
			return true;
		return false;
	};

	for (;;) {
		if (!*str) 
			return res;
		if (rd::String::isWhitespace(*str)) {
			str++;
			continue;
		}
		else if (hasKeyword(str, "true")) {
			res.push_back(Tok(TokenKind::True));
			str += strlen("true");
			continue;
		}
		else if (hasKeyword(str, "false")) {
			res.push_back(Tok(TokenKind::False));
			str += strlen("false");
			continue;
		}
		else if (hasKeyword(str, "and")) {
			res.push_back(Tok(TokenKind::LogicalAnd));
			str += strlen("and");
			continue;
		}
		else if (hasKeyword(str, "or")) {
			res.push_back(Tok(TokenKind::LogicalOr));
			str += strlen("or");
			continue;
		}
		else 
		switch (*str) {
			case '<': {
				if (*str && (*(str + 1) == '=')) {
					res.push_back(Tok(TokenKind::Le));
					str++;
				}
				else
					res.push_back(Tok(TokenKind::Lt));
				str++;
				break;
			}
			case '>': {
				if (*str && (*(str + 1) == '=')) {
					res.push_back(Tok(TokenKind::Ge));
					str++;
				}
				else
					res.push_back(Tok(TokenKind::Gt));
				str++;
				break;
			}
			case '=': {
				if (*str && (*(str + 1) == '=')) {//intercept ==
					res.push_back(Tok(TokenKind::Eq));
					str++;
				}
				else 
					res.push_back(Tok(TokenKind::Eq));
				str++;
				break;
			}
			case '!': {
				if (*str && (*(str + 1) == '=')) {
					res.push_back(Tok(TokenKind::NEq));
					str++;
				}
				else
					res.push_back(Tok(TokenKind::ExclamationMark));
				str++;
				break;
			}
			case '|': {
				if (*str && (*(str + 1) == '|')) {
					res.push_back(Tok(TokenKind::LogicalOr));
					str++;
				}
				else
					res.push_back(Tok(TokenKind::BitwiseOr));
				str++;
				break;
			}
			case '&': {
				if (*str && (*(str + 1) == '&')) {
					res.push_back(Tok(TokenKind::LogicalAnd));
					str++;
				}
				else
					res.push_back(Tok(TokenKind::BitwiseAnd));
				str++;
				break;
			}
			case ',': {
				res.push_back(Tok(TokenKind::Comma));
				str++;
				break;
			}
			case '(': {
				res.push_back(Tok(TokenKind::ParOpen));
				str++;
				break;
			}
			case ')': {
				res.push_back(Tok(TokenKind::ParClose));
				str++;
				break;
			}
			case ':': {
				res.push_back(Tok(TokenKind::Colon));
				str++;
				break;
			}
			case '+': {
				res.push_back(Tok(TokenKind::OpPlus));
				str++;
				break;
			}
			
			case '-': {
				res.push_back(Tok(TokenKind::OpMinus));
				str++;
				break;
			}
			
			case '*': {
				res.push_back(Tok(TokenKind::OpMul));
				str++;
				break;
			}
			case '/': {
				res.push_back(Tok(TokenKind::OpDiv));
				str++;
				break;
			}
			case '"': {
				str++;
				auto end = rd::String::skipTo(str,'\"');
				if (!end)
					return res;
				auto lit = rd::String::sub(str, end);
				int here = 0;
				res.push_back(TokDataS(TokenKind::String, lit.c_str()));
				str = end + 1;
				break;
			}
			default: {
				if (rd::String::isNewline(*str)) {
					str++;
				}

				bool parsedNum = false;
				if (rd::String::isNumFloat(*str)) {//starts with a float alike token
					const char * toControlToken = rd::String::skipToParenOrComma(rd::String::skipWhitespace(str));
					Str numlit = rd::String::sub(str, toControlToken);
					bool forceFloat = false;
					if (rd::String::hasFloatSymbols(numlit.c_str())) 
						forceFloat = true;
					if (!forceFloat && rd::String::isInteger(numlit.c_str())) {
						const char* start = str;
						const char* end = str;
						double fl = rd::String::parseInt(start, 0, end);
						res.push_back(TokDataI(TokenKind::Int, fl));
						if (end)
							str = end;
						parsedNum = true;
					}
					else if (rd::String::isFloat(str)) {
						const char* start = str;
						const char* end = str;
						double fl = rd::String::parseFloat(start, 0.0, end);
						res.push_back(TokDataD(TokenKind::Float, fl));
						if (end)
							str = end;
						parsedNum = true;
					}
				}
				if(!(parsedNum)) {
					/*
					if( str[0] == '-' ) {
						res.push_back(Tok(TokenKind::OpMinus));
						str++;
						break;
					}
					else*/
					if (rd::String::isIdentifierCharacterEmojiCompat(*str)) {
						const char* start = str;
						const char* end = str;
						Str id = rd::String::parseIdentifier(start);
						str += id.length();
						res.push_back(TokDataS(TokenKind::Identifier, id.c_str()));
					}
					else
						str++;
				}
				break;
			}
		}
	}

	return res;
}


static Token	s_dummyToken;

CondNode* CondParser::parse( CondLexer & lex, int pos, int& outputNext) {
	if (pos >= lex.tokens.size()) {
		success = False;//?
		return new CondNode(&s_dummyToken);//expression is probably incomplete
	}
	auto isEOF = [&](int _pos) {
		return _pos >= lex.tokens.size();
	};
	auto isNextToken = [&](TokenKind tk ) {
		return lex.tokens[pos+1].kind == tk;
	};
	auto isToken = [&](TokenKind tk ) {
		return lex.tokens[pos].kind == tk;
	};
	auto& tok = lex.tokens[pos];
	switch (tok.kind) {
		case TokenKind::None:				return new CondNode( &s_dummyToken);	break;
		case TokenKind::ParOpen: {
			auto nc = new CondNode(&tok);
			pos++;
			outputNext = pos;
			nc->l = parseAndResume(lex, pos, outputNext);
			return nc;
			break;
		}

		case TokenKind::ParClose:	//do nothing?
		{
			pos++;
			outputNext=pos;
			return new CondNode(&s_dummyToken);
			break;
		}

		case TokenKind::QuestionMark: {
			auto cd = new CondNode(&tok);
			cd->kind |= CondNodeKind::ControlTernary;
			int next = pos + 1;
			cd->pred = parse(lex, pos + 1, next);
			cd->l = parse(lex, next, next);
			if (lex.tokens[next].kind == TokenKind::Colon) {
				next++;
				cd->r = parse(lex, next, next);
			}
			outputNext = next + 1;
			return cd;
		}
		case TokenKind::ExclamationMark: {
			auto cd = new CondNode(&tok);
			cd->kind |= CondNodeKind::ControlUnary;
			cd->l = parse(lex, pos + 1, outputNext);
			outputNext++;
			return cd;
		}
		case TokenKind::Comma:				
			break;
		case TokenKind::Colon:				
			break;
		case TokenKind::Identifier: {
			bool isFuncall = false;
			if (!isEOF(pos+1)) {
				if (isNextToken(TokenKind::ParOpen)) {
					pos++;
					auto cd = new CondNode(&tok);
					cd->kind |= CondNodeKind::Funcall;
					pos++;
					for (;;) {
						if (isToken(TokenKind::ParClose)) {
							pos++;
							outputNext = pos;
							return cd;
						}
						else {
							outputNext = pos;
							auto arg = parse(lex, pos, outputNext);
							cd->args.push_back( arg );
							pos = outputNext;
							if (isEOF(pos))
								return cd;

							if (isToken(TokenKind::Comma)) {
								pos++;
								outputNext = pos;
							}
						}
						if (isEOF(pos)) 
							return cd;
					}
					return cd;
				}
			}

			if (!isFuncall) {//regular "name" that can be replaced or queried 
				outputNext++; 
				return new CondNode(&tok);
			}
			break;
		}
		case TokenKind::String:				outputNext++; return new CondNode(&tok); break;
		case TokenKind::Float:				outputNext++; return new CondNode(&tok); break;
		case TokenKind::Int:				outputNext++; return new CondNode(&tok); break;
		case TokenKind::Str:				outputNext++; return new CondNode(&tok); break;
		case TokenKind::BitwiseOr:	//[[fallthrough]]			
		case TokenKind::BitwiseAnd:	//[[fallthrough]]	
		case TokenKind::LogicalOr:	//[[fallthrough]]			
		case TokenKind::LogicalAnd:	//[[fallthrough]]	
		case TokenKind::OpPlus:		//[[fallthrough]]
		case TokenKind::OpMinus:	//[[fallthrough]]
		case TokenKind::OpMul:		//[[fallthrough]]			
		case TokenKind::OpDiv:		//[[fallthrough]]			
		case TokenKind::Gt:			//[[fallthrough]]			
		case TokenKind::Ge:			//[[fallthrough]]			
		case TokenKind::Lt:			//[[fallthrough]]			
		case TokenKind::Le:			//[[fallthrough]]			
		case TokenKind::Eq:			//[[fallthrough]]			
		case TokenKind::NEq:		//[[fallthrough]]			
		{
			auto cd = new CondNode(&tok);
			cd->kind |= CondNodeKind::ControlBinary;
			//cd->l //let the parser push first
			outputNext++;
			cd->r = parse(lex, pos + 1, outputNext);
			return cd;
		}
		case TokenKind::True:				outputNext++; return new CondNode(&tok); break;
		case TokenKind::False:				outputNext++; return new CondNode(&tok); break;
	};
	return new CondNode(&s_dummyToken); 
}

rd::parse::CondNode* rd::parse::CondParser::parseAndResume(CondLexer& lex, int pos, int& outputNext) {
	rd::parse::CondNode* first = parse(lex, pos, outputNext);
	bool shouldResume = false;
	pos = outputNext;
	while(pos < lex.tokens.size()) {
		rd::parse::CondNode* next = parse(lex, pos, outputNext);
		pos = outputNext;
		if (next) {
			if (bool(next->kind & CondNodeKind::ControlBinary)) {
				{
					//suspend add/min and priorize mul/div
					bool isMul = (next->t->kind == TokenKind::OpMul) || (next->t->kind == TokenKind::OpDiv);
					if (isMul) {
						auto lop = first->r;
						next->l = first->r;
						first->r = next;
						int here = 0;
					}
					else {
						next->l = first;
						first = next;
					}
				}
			}
			if (next->kind == CondNodeKind::None)
				break;
		}
	}
	return first;
}

void rd::parse::CondParser::parse(){
	int next = 0;
	root = parseAndResume(lex,0,next);
	//we don't care about next here it is probably something close to an end of file
}

CondParser* rd::parse::CondParser::parse(const char* str){
	CondParser* cp = new CondParser();
	cp->lex = CondLexer::parse(str);
	cp->parse();
	return cp;
}

void rd::parse::CondParser::im(){
	using namespace ImGui;
	if (!root)
		ImGui::Text("Cannot parse "s + lex.debug.c_str());
	else {
		ImGui::Text(lex.debug.c_str());
		Indent();
		root->im();
		Unindent();
	}
}

void rd::parse::CondNode::im(){
	using namespace ImGui;
	typedef rd::parse::CondNodeKind Cond;

	#define DO( Ident )  if( (bool) (kind & Ident) ) Text( # Ident );

	DO( Cond::None )
	DO( Cond::Expr )
	DO( Cond::Funcall )
	DO( Cond::ControlUnary )
	DO( Cond::ControlBinary )
	DO( Cond::ControlTernary )
	
	#undef DO
	t->im();

	if (l) {
		Indent();
		l->im();
		Unindent();
	}
	//else
	//	Text("No left leaf");

	if (r) {
		Indent();
		r->im();
		Unindent();
	}

	if (args.size()) {
		Indent();
		for(auto&a:args)
			a->im();
		Unindent();
	}
	//else
	//	Text("No right leaf");
}


std::any rd::parse::CondParser::evalOp(CondContext& ctx) {
	if (!root) return {};
	return eval(ctx, *root);
}

static bool isFalse(std::any& res) {
	if (res.type() == typeid(int))
		return std::any_cast<int>(res) == 0;
	else if (res.type() == typeid(float))
		return std::any_cast<float>(res) == 0.0f;
	else if (res.type() == typeid(double))
		return std::any_cast<double>(res) == 0;
	else if (res.type() == typeid(bool))
		return std::any_cast<bool>(res) == 0;
	else if (res.type() == typeid(bool))
		return std::any_cast<bool>(res) == 0;
	else if (res.type() == typeid(std::string)) {
		auto& str = std::any_cast<std::string>(res);
		if (rd::String::equalsI(str.c_str(), "0"))
			return false;
		if ( rd::String::equalsI(str.c_str(), "false"))
			return false;
		if (str.empty())
			return false;
		return true;
	}
	return false;
}

static double getNumeralDouble(std::any& val) {
	if (val.type() == typeid(int))
		return (double) std::any_cast<int>(val);
	if (val.type() == typeid(float))
		return (double)std::any_cast<float>(val);
	if (val.type() == typeid(double))
		return std::any_cast<double>(val);
	if (val.type() == typeid(bool))
		return (double) std::any_cast<bool>(val);
	return 0.0;
}

static int getNumeralInt(std::any& val) {
	if (val.type() == typeid(int))
		return std::any_cast<int>(val);
	if (val.type() == typeid(float))
		return (int)std::any_cast<float>(val);
	if (val.type() == typeid(double))
		return (int)std::any_cast<double>(val);
	if (val.type() == typeid(bool))
		return (int)std::any_cast<bool>(val);
	return 0;
}

static bool is_numeral(std::any& any_obj) {
	return (
				(any_obj.type() == typeid(int))
			||	(any_obj.type() == typeid(float)) 
			||	(any_obj.type() == typeid(double))
			||	(any_obj.type() == typeid(bool))
	);
}

static bool any_eq(std::any& l, std::any& r) {
	#define cmp( ty ) if( (l.type() == typeid(ty)) && (r.type() == typeid(ty)) ) return std::any_cast<ty>(l) == std::any_cast<ty>(r);

	cmp(int)
	cmp(float)
	cmp(double)
	cmp(std::string)
	cmp(bool)

	trace(to_string(l) +" == " + to_string(r));
	if (is_numeral(l) && is_numeral(r)) {
		double lv = getNumeralDouble(l);
		double rv = getNumeralDouble(r);
		return r::Math::approximatelyEqual(lv, rv);
	}

	#undef cmp
	#undef is_numeral
		
	return false;
}


static bool any_lt(std::any& l, std::any& r) {
	#define cmp( ty ) if( (l.type() == typeid(ty)) && (r.type() == typeid(ty)) ) return std::any_cast<ty>(l) < std::any_cast<ty>(r);

	cmp(int)
	cmp(float)
	cmp(double)
	cmp(std::string)
	cmp(bool)

	double fbckL = 0.0f;
	double fbckR = 0.0f;

	if (is_numeral(l) && is_numeral(r)) 
		return getNumeralDouble(l) < getNumeralDouble(r);

	#undef cmp

	return false;
}

bool rd::parse::CondParser::eval(CondContext& ctx) {
	if (!root) return false;
	auto res = eval(ctx, *root);
	return !isFalse(res);//final bool comp
}

std::any rd::parse::CondParser::eval(CondContext& ctx,  CondNode & cur) {
	auto& tk = cur.t->kind;
	if ((bool)(cur.kind & CondNodeKind::ControlUnary)) {
		if (cur.t->kind == TokenKind::ExclamationMark) {
			if (!cur.l)
				return false;
			auto res = eval(ctx,  *cur.l);
		}
		else if (cur.t->kind == TokenKind::OpMinus) {
			if (!cur.l)
				return false;
			auto res = eval(ctx,  *cur.l);
			trace(to_string(res));
			return - getNumeralInt(eval(ctx,  *cur.l));
		}
	}
	else if ((bool)(cur.kind & CondNodeKind::ControlBinary)) {
		int here = 0;

		switch (cur.t->kind) {
			case TokenKind::LogicalOr:{
				auto resL = eval(ctx,  *cur.l);
				if (!isFalse(resL))
					return true;
				auto resR = eval(ctx,  *cur.r);
				if (!isFalse(resR))
					return true;
				return false;
			}
			break;
			case TokenKind::LogicalAnd: {
				if (!cur.l)
					return false;
				auto resL = eval(ctx,  *cur.l);
				if (isFalse(resL))
					return false;
				if (!cur.r)
					return resL;
				auto resR = eval(ctx, *cur.r);
				return any_eq(resL, resR);
			}
			break;
			case TokenKind::BitwiseAnd: {
				if (!cur.l) return false;
				if (!cur.r) return false;
				int lv = getNumeralInt(eval(ctx, *cur.l));
				int rv = getNumeralInt(eval(ctx, *cur.r));
				return (lv & rv);
			}
			break;
			case TokenKind::BitwiseOr: {
				if (!cur.l) return false;
				if (!cur.r) return false;
				int lv = getNumeralInt(eval(ctx,  *cur.l));
				int rv = getNumeralInt(eval(ctx,  *cur.r));
				return (lv | rv);
			}
			case TokenKind::Gt: {
				if (!cur.l) return false;
				if (!cur.r) return false;
				auto lr = eval(ctx, *cur.l);
				auto rr = eval(ctx, *cur.r);
				if (any_eq(lr, rr))
					return false;
				if (any_lt(lr, rr))
					return false;
				return true;
			}
			case TokenKind::Ge: {
				if (!cur.l) return false;
				if (!cur.r) return false;
				return !any_lt(
					eval(ctx,  *cur.l),
					eval(ctx,  *cur.r));
			}
			case TokenKind::Lt: {
				if (!cur.l) return false;
				if (!cur.r) return false;
				return any_lt(
					eval(ctx,  *cur.l),
					eval(ctx,  *cur.r));
			}
			case TokenKind::Le: {
				if (!cur.l) return false;
				if (!cur.r) return false;
				auto lr = eval(ctx,  *cur.l);
				auto rr = eval(ctx,  *cur.r);
				if (any_lt(lr,rr))
					return true;
				if (any_eq(lr, rr))
					return true;
				return false;
			}
			case TokenKind::Eq: {
				if (!cur.l) return false;
				if (!cur.r) return false;
				auto lr = eval(ctx,  *cur.l);
				auto rr = eval(ctx,  *cur.r);
				return any_eq(lr, rr);
			}
			case TokenKind::NEq: {
				if (!cur.l) return false;
				if (!cur.r) return false;
				auto lr = eval(ctx,  *cur.l);
				auto rr = eval(ctx,  *cur.r);
				return !any_eq(lr, rr);
			}
			case TokenKind::OpPlus: {
				if (!cur.l) return false;
				if (!cur.r) return false;
				auto lr = eval(ctx, *cur.l);
				auto rr = eval(ctx, *cur.r);
				return getNumeralDouble(lr) + getNumeralDouble(rr);
			}
			case TokenKind::OpMinus: {
				if (cur.l && cur.r) {
					auto lr = eval(ctx, *cur.l);
					auto rr = eval(ctx, *cur.r);
					return getNumeralDouble(lr) - getNumeralDouble(rr);
				}
				else {
					if (cur.l) {
						auto lr = eval(ctx, *cur.l);
						return -getNumeralDouble(lr);
					}
					else if( cur.r ){
						auto lr = eval(ctx, *cur.r);
						return -getNumeralDouble(lr);
					}
				}
			}
			case TokenKind::OpMul: {
				if (!cur.l) return false;
				if (!cur.r) return false;
				auto lr = eval(ctx, *cur.l);
				auto rr = eval(ctx, *cur.r);
				return getNumeralDouble(lr) * getNumeralDouble(rr);
			}
			case TokenKind::OpDiv: {
				if (!cur.l) return false;
				if (!cur.r) return false;
				auto lr = eval(ctx, *cur.l);
				auto rr = eval(ctx, *cur.r);
				return getNumeralDouble(lr) / getNumeralDouble(rr);
			}
		}
	}
	else if ((bool)(cur.kind & CondNodeKind::ControlTernary)) {
		switch (cur.t->kind) {
			case TokenKind::QuestionMark: {
				if (!cur.pred) return false;
				if (!cur.l) return false;
				if (!cur.r) return false;
				auto pr = eval(ctx, *cur.pred);
				auto lr = eval(ctx, *cur.l);
				auto rr = eval(ctx, *cur.r);
				if (!isFalse(pr))
					return lr;
				else
					return rr;
				break;
			}
			default:
				return false;
		}
	}
	else if ((bool)(cur.kind & CondNodeKind::Funcall)) {
		if (ctx.onFuncall) {
			const char* fname = cur.t->sdata.c_str();
			eastl::vector<std::any> args;
			for (auto a : cur.args)
				if (a)
					args.push_back(eval(ctx,  *a));
			return ctx.onFuncall(fname, args);
		}
		else if (ctx.onFuncallAst)
			return ctx.onFuncallAst(cur);
		else
			return false;
	}
	else if ((bool)(cur.kind & CondNodeKind::Expr)) {
		switch (tk) {
			case TokenKind::None:		return false;
			case TokenKind::Identifier: return ctx.onSingleIdentifier(cur.t->sdata.c_str());
			case TokenKind::Float:		return cur.t->data.dval;
			case TokenKind::Int:		return cur.t->data.ival;
			case TokenKind::True:		return 1;
			case TokenKind::False:		return 0;
			case TokenKind::String:
				return cur.t->sdata;
			case TokenKind::ParOpen:
				if (cur.l) {
					return eval(ctx, *cur.l);
				}
				else {
					traceError("error evaluating paren "s + std::to_string((int)cur.kind));
					return false;
				}
			default:
				traceError("error evaluating condition "s + std::to_string((int)cur.kind));
				auto here = "todo";
				break;
		}
	}
	return {};
}

rd::parse::CondContext::CondContext() {
	onFuncall			= [](auto, auto) {return false; };
	onFuncallAst		= [](auto) {return false; };
	onSingleIdentifier	= [](auto) {return false; };
}
