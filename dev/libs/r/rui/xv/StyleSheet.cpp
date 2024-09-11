
#include "stdafx.h"

#include "Str.h"
#include "rd/Style.hpp"
#include "StyleSheet.hpp"

using namespace rui;
using namespace rui::xv;

rui::xv::StyleSheet::StyleSheet(){

}

rui::xv::StyleSheet::~StyleSheet() {

}


bool rui::xv::StyleSheet::parse(char* head){
	StyleSheetLexer lex(head);
	parseSymbols(lex);
	lexer = lex;
	return true;
}

static int skipWhitespaces(StyleSheetLexer& lex, int idx) {
	while (lex.buf[idx].id == Spaces)
		idx++;
	return idx;
}

bool rui::xv::StyleSheet::parseSymbols(StyleSheetLexer& lex, int idx) {
	Selector sel;

	for (;;) {
		idx = skipWhitespaces(lex, idx);
		int nextIdx = parseSelector(lex, idx, sel);
		if (nextIdx == -1)
			return false;
		idx = skipWhitespaces(lex, idx);

		rd::Style* ns = new rd::Style();
		int afterBlockIdx = parseBlock(lex, nextIdx, *ns);
		if (afterBlockIdx == -1) {
			delete ns;
			return true;
		}
		idx = skipWhitespaces(lex, afterBlockIdx);
		
		StyleSheetEntry se;
		se.sel = sel;
		se.st = ns;
		addStyle(se);

		sel = {};
	}
}

static int parseID(StyleSheetLexer& lex, int idx, Str& name) {
	if (lex.buf[idx].id == Sharp) {
		idx++;
		if (lex.buf[idx].id != Ident) {
			lex.errorLog = "unexpected char after #";
			return -1;
		}
		name = rd::String::sub(lex.buf[idx].dataHead, lex.buf[idx].dataEnd);
		idx++;
	}
	return idx;
}

static int parseClass(StyleSheetLexer& lex, int idx, Selector& sel) {
	if (lex.buf[idx].id == Dot) {
		idx++;
		if (lex.buf[idx].id != Ident) {
			lex.errorLog = "unexpected char after .";
			return -1;
		}
		Str cl = rd::String::sub(lex.buf[idx].dataHead, lex.buf[idx].dataEnd);
		sel.classes.push_back(cl);
		idx++;
	}
	return idx;
}

int rui::xv::StyleSheet::parseSelector(StyleSheetLexer& lex, int idx, Selector& sel){
	idx = skipWhitespaces(lex, idx);

	idx = parseID(lex,idx,sel.id);
	if (idx < 0) return -1;
	
	for (;;) {
		idx = skipWhitespaces(lex, idx);
		if (lex.buf[idx].id != Dot)
			break;
		idx = parseClass(lex, idx, sel);
		idx = skipWhitespaces(lex, idx);
	}

	return idx;
}

int rui::xv::StyleSheet::parseBlock(StyleSheetLexer& lex, int idx, rd::Style& st){
	if (lex.buf[idx].id != BrOpen)
		return -1;
	idx++;

	auto skip = [&]() {
		idx = skipWhitespaces(lex, idx);
	};

	auto is = [&]( const auto& i) {
		return lex.buf[idx].id == i;
	};

	Str lval;
	Str rval;
	//Str unitVal;
	int nbRead = 0;
	for(;;)
	{
		skip();
		if (!is(Ident)) {
			lex.errorLog = "expected ident";
			return -1;
		}
		else
			lval = rd::String::sub(lex.buf[idx].dataHead, lex.buf[idx].dataEnd);
		idx++; skip();
		if( !is(Colon) && !is(Equal)) {
			lex.errorLog = "expected Colon";
			return -1;
		}
		idx++; skip();
		if ( !is(Ident) && !is(Int) && !is(Float)) {
			lex.errorLog = "expected something readable";
			return -1;
		}
		else {
			if (is(Ident)) {
				lex.errorLog = "ident not expected";
				return -1;
			}
			else if (is(Float)) {
				st( lval.c_str(), (float) rd::String::parseFloat( lex.buf[idx].extractLiteral().c_str() ) );
			}
			else if (is(Int)) {
				st(lval.c_str(), rd::String::parseInt(lex.buf[idx].extractLiteral().c_str()));
			}
		}
		idx++; skip();
		if (is(Ident)) {//this could be a unit but we won't process it for now
			lex.errorLog = "unit not expected";
			return -1;
		}
		skip();
		if (!is(Semicolon)) {//parse ok
			if (is(BrClose)) {
				idx++;
				break;
			}
			else {
				lex.errorLog = "missing ; ?";
				return -1;
			}
		}
		
		idx++;
		skip();
		if (is(BrClose)) {
			idx++;
			break;
		}
		else
			nbRead++;
	}
	skip();
	return idx;
}

void rui::xv::StyleSheet::addStyle(StyleSheetEntry& se){
	styleRules.push_back(se);
}

bool rui::xv::StyleSheet::parseSymbols(StyleSheetLexer& lex){
	return parseSymbols(lex,0);
}

rui::xv::StyleSheetEntry::StyleSheetEntry(){
	st = new rd::Style();
}

rui::xv::StyleSheetLexer::StyleSheetLexer(char* str){
	head = str;
	buf.reserve(strlen(str));
	lex(str);
}

void rui::xv::StyleSheetLexer::lex(char* str){
	if (!str) return;

	for (;;) {
		if (!*str) {
			buf.push_back(Eof);
			return;
		}

		switch(*str) {
			case '%': {
				buf.push_back(Tok(Percent));
				str++;
				break;
			}
			case '!': {
				buf.push_back(Tok(Exclam));
				str++;
				break;
			}
			case '#': {
				buf.push_back(Tok(Sharp));
				str++;
				break;
			}
			case '.': {
				buf.push_back(Tok(Dot));
				str++;
				break;
			}
			case ',': {
				buf.push_back(Tok(Comma));
				str++;
				break;
			}
			case '(': {
				buf.push_back(Tok(POpen));
				str++;
				break;
			}
			case ')': {
				buf.push_back(Tok(PClose));
				str++;
				break;
			}
			case '{': {
				buf.push_back(Tok(BrOpen));
				str++;
				break;
			}
			case '}': {
				buf.push_back(Tok(BrClose));
				str++;
				break;
			}
			case ':': {
				buf.push_back(Tok(Colon));
				str++;
				break;
			}
			case ';': {
				buf.push_back(Tok(Semicolon));
				str++;
				break;
			}
			case ' ': {
				buf.push_back(Tok(Spaces));
				str++;
				break;
			}
			//case '-': {
			//	buf.push_back(Tok(Dash));
			//	str++;
			//	break;
			//}
			case '=': {
				buf.push_back(Tok(Equal));
				str++;
				break;
			}
			case '/': {
				//buf.push_back(Tok(Slash));
				//str++;
				auto n = str + 1;
				bool isNextStar = *n == '*';
				bool isNextSlash = *n == '/';
				if (!isNextStar && !isNextSlash) {
					buf.push_back(Tok(Slash));
					str++;
					break;
				}

				if (isNextSlash) {
					auto c = n+1;
					for (;;) {
						if(rd::String::isNewline(*c))
							break;
						if (*c == 0) {
							buf.push_back(Tok(Eof));
							str++;
							break;
						}
						c++;
					}
					break;
				}
				else {//implicit isNextStar
					auto c = n + 1;
					while (true) {
						while ( *c != '*') {
							if (*c == 0) {
								errorLog = "Unclosed comment";
								return;
							}
						}
						c = c + 1;
						if (*c == '/') break;
						if (!*c) {
							errorLog = "Unclosed comment";
							return;
						}
					}
				}
				break;
			}
			case '\\': {
				buf.push_back(Tok(AntiSlash));
				str++;
				break;
			}
			case '*': {
				buf.push_back(Tok(Star));
				str++;
				break;
			}
			default: {
				
				if (rd::String::isNewline(*str)) {
					buf.push_back(Tok(Spaces));
					str++;
				}
				else if (rd::String::isFloat(str)) {
					buf.push_back(Tok(Float, str));
					while (*str) {
						if (!*str) {
							buf.push_back(Tok(Eof));
							return;
						}
						if (rd::String::isNumFloat(*str))
							str++;
						else {
							buf[buf.size() - 1].dataEnd = str;
							break;
						}
					}
				}
				else if (rd::String::isInteger(str)) {
					buf.push_back(Tok(Int, str));
					while (*str) {
						if (!*str) {
							buf.push_back(Tok(Eof));
							return;
						}
						if (rd::String::isNum(*str))
							str++;
						else {
							buf[buf.size() - 1].dataEnd = str;
							break;
						}
					}
				}
				else if (rd::String::isIdentifierCharacterEmojiCompat(*str)) {
					buf.push_back(Tok(Ident, str));
					while (*str) {
						if (!*str) {
							buf.push_back(Tok(Eof));
							return;
						}
						if (rd::String::isAlphaNum(*str))
							str++;
						else {
							buf[buf.size() - 1].dataEnd = str;
							break;
						}
					}
				}
				else 
				break;
			}
		}
	}
}

bool StyleSheet::im() {
	using namespace ImGui;
	bool chg = false;

	int idx = 0;
	for (auto& s : styleRules) {
		if (TreeNode( std::to_string(idx) +" "+ std::to_string(s.sel) )) {
			s.im();
			TreePop();
		}
		idx++;
	}
	if (TreeNode("Lexer")) {
		lexer.im();
		TreePop();
	}

	return chg;
}

void Selector::im() {
	using namespace ImGui;
	Value("id",id);
	for (auto&cl: classes) {
		Text(cl);
		SameLine();
	}
	NewLine();
}

void StyleSheetEntry::im() {
	using namespace ImGui;
	Text("Sel");
	sel.im();
	if(st) st->im();
}

void StyleSheetLexer::im() {
	using namespace ImGui;
	if (errorLog.length())
		Error(errorLog);
	Text("Sel");
	for (auto& v : buf) 
		v.im();
	Text("head", head);
}

void rui::xv::Tok::im(){
	using namespace ImGui;

#define DO( Ident )  case Ident: Text( # Ident ); break;
	switch (id){
		DO(Ident			 )
		DO(String			 )
		DO(Int				 )
		DO(Float			 )
		DO(Colon			 )
		DO(Sharp			 )
		DO(POpen			 )
		DO(PClose			 )
		DO(Exclam			 )
		DO(Comma			 )
		DO(Percent			 )
		DO(Semicolon		 )
		DO(BrOpen			 )
		DO(BrClose			 )
		DO(Dot				 )
		DO(Spaces			 )
		//DO(Dash			 )
		DO(Slash			 )
		DO(AntiSlash		 )
		DO(Star				 )
		DO(Eof				 )
	default:
		break;
	}
	
	if(dataHead)
		Value("literal", rd::String::sub(dataHead, dataEnd));
}
#undef DO

Str Tok::extractLiteral() {
	return !dataHead ? "" : rd::String::sub(dataHead, dataEnd);
};

static std::string std::to_string(const rui::xv::Selector& sel) {
	std::string res;
	if (sel.id.length())
		res = res + "#" + sel.id.cpp_str();
	for (auto& cl : sel.classes) 
		res = res + "." + cl.cpp_str();
	return res;
};

eastl::vector<StyleSheetEntry*> StyleSheet::infer(const Selector& sel) {
	eastl::vector<StyleSheetEntry*> res;
	for (auto& st : styleRules) 
		if (st.sel.matches(sel))
			res.push_back(&st);
	return res;
}

void StyleSheet::append(const StyleSheet& se) {
	for (auto& st : se.styleRules) {
		styleRules.push_back(st);
	}
}

