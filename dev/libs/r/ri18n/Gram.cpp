#include "stdafx.h"
#include "../utf8/utf8.h"
#include "Lex.hpp"
#include "Gram.hpp"
#include "Nar.hpp"

#include <vector>
#include <iostream>

using namespace std;
using namespace ri18n;
using ri18n::AstNodeType;

string fromCharCode(int code) {
	char cc[4] = {};
	utf8catcodepoint(cc, code, 4);
	return string(cc);
}

bool isChar(const Lexem& l) {
	switch (l.type) {
		case Char:	return true;	
		default:		return false;
	};
}

int getChar(const Lexem& l) {
	switch (l.type) {
		case Char:		return l.cc;
		default:		return 0;
	}
}

string getCharString(const Lexem& l) {
	switch (l.type) {
		case Char:		return fromCharCode(l.cc);
		default:		return 0;
	}
}

bool isRestEmpty(AstNode* rest) {
	if (!rest) return false;

	switch (rest->type) {
		case AstNodeType::Nop: return true;
		default: return false;
	}
}

AstNode * Gram::parse(const vector<Lexem>& str) {
	return parse(str, 0);
}

string restring(const vector<Lexem>& str) {
	return Lex::lexemsToString(str);
}

string restringOne(const Lexem& elem) {
	return Lex::lexemToString(elem);
}

AstNode * mkSeq(AstNode * ast, AstNode * rest) {
	if (rest == nullptr) {
		return ast;
	}
	else if (ast == nullptr) {
		return rest;
	}
	else 
		return new AstNode(AstNodeType::Seq, ast, rest);
}

static string getElementType(const string & str) {
	auto spacePos = str.find(" ");
	if (spacePos == -1) 
		return str;
	else 
		return str.substr(0,spacePos);
}

int seek(const vector<Lexem>& str, int start, const Lexem& tok ) {
	int res = -1;
	for (int i = start ;i< str.size();++i)
		if (str[i].type == tok.type && str[i].strData == tok.strData) {
			res = i;
			break;
		}
	return res;
}

AstNode * Gram::parse(const vector<Lexem>& str, int pos) {
	if (pos >= str.size())
		return nullptr;

	const Lexem& l = str[pos];
	switch (l.type) {
		case BrackPosOpen:{
			int res = -1;
			for (int i = pos + 1; i < str.size();++i)
				if (str[i].type == BrackClose) {
					res = i;
					break;
				}
			if (res == -1) {
#ifdef _DEBUG
				cerr << "ri8n::WARNING::suspicious brack close missing" << Lex::lexemsToString(str) << endl;
#endif
				return mkSeq( new AstNode(AstNodeType::Sentence,"[>"), parse(str, pos + 1));
			}
			else {
				vector<Lexem> sub(str.begin() + pos + 1, str.begin() + res);
				AstNode * rest = parse(str, res + 1);
				return mkSeq( new AstNode(AstNodeType::TagFrom,restring(sub), isRestEmpty(rest)), rest);
			}
		}
		break;
		case Char:{
			string b;
			int pp = pos;
			while (pp < str.size() && isChar(str[pp])) {
				b.append(getCharString(str[pp]));
				pp++;
			}
			if (pp >= str.size())
				return new AstNode(AstNodeType::Sentence, b);
			else
				return mkSeq(new AstNode(AstNodeType::Sentence, b), parse(str, pp));
		}
		case TagSelfClosed: {
			//string trimmed = rd::String::trim(l.strData);
			if (l.strData == "br") {
				int rest = pos + 1 - str.size();
				auto n = new AstNode(AstNodeType::Sentence, "\n");
				if (rest == 0) return n;
				return mkSeq(n, parse(str, pos + 1));
			}
			else {
				auto a0 = new AstNode(AstNodeType::Tag, l.strData);
				int rest = pos + 1 - str.size();
				if (rest == 0)
					return a0;
				return mkSeq(a0, parse(str, pos + 1));
			}
		}

		case Literal:
			return new AstNode(AstNodeType::Sentence,l.strData);

		case TagOpen: {
			string elementType = getElementType(l.strData);
			
			int res = seek(str, pos + 1, Lexem(TagClose, elementType));
			if (res == -1) {
#ifdef _DEBUG
				cerr << "ri8n::WARNING::suspicious tag close missing" << Lex::lexemsToString(str) << endl;
#endif
				return mkSeq(new AstNode(AstNodeType::Sentence, restringOne(str[pos])), parse(str, pos + 1));
			}

			AstNode * content = parse(vector<Lexem>(str.begin() + pos + 1, str.begin() + res), 0);
			AstNode * rest = parse(str, res + 1);
			return mkSeq(new AstNode(AstNodeType::Tag, l.strData, content), rest);
		}
		break;

		//[Fallthrough]
		case AccClose:
		case BrackClose:
		case TagClose:
			return mkSeq( new AstNode(AstNodeType::Sentence,restringOne(str[pos])), parse(str, pos + 1));
		break;

		case Star:
		case DoubleStar:
		case Pipe:
		case DoubleSemiColon:{
			const char * lit= nullptr;
			switch(l.type) {
				case Star: lit= "*"; break;
				case DoubleStar: lit="**"; break;
				case Pipe: lit="|"; break;
				case DoubleSemiColon: lit="::"; break;
				default: lit = "error"; break;
			}

			auto closure = [&l,this](vector<Lexem> & str){
				switch (l.type) {
					case Star:			return new AstNode(AstNodeType::Em,parse(str, 0));
					case DoubleStar:	return new AstNode(AstNodeType::Strong,parse(str, 0));
					case DoubleSemiColon:
					case Pipe: 
										return new AstNode(AstNodeType::Script,restring(str));
					default: 
										return new AstNode(AstNodeType::Sentence,restring(str));
				}
			};
			int res = -1;
			for ( int i = pos + 1; i < str.size(); ++i)
				if (str[i].type == l.type && str[i].strData == l.strData) {
					res = i;
					break;
				}
			if (res == -1) {
#ifdef _DEBUG
				cerr << "ri8n::WARNING::suspicious pair broken " << Lex::lexemsToString(str)<<"\n";
#endif
				return mkSeq(new AstNode(AstNodeType::Sentence,lit), parse(str, pos + 1));
			}
			else {
				vector<Lexem> sub( str.begin() + pos + 1, str.begin() + res);
				return mkSeq( closure(sub), parse(str, res + 1));
			}
		}
		break;

		//[Fallthrough]
		case BrackOpen:
		case BrackCondOpen:
		case AccOpen:
		case BrackImportantOpen:
		case AccCondOpen:
		{
			LexemType close;
			switch (l.type) {
			default:throw "assert";
			case BrackOpen:		close = BrackClose; break;
			case AccOpen:		close = AccClose; break;
			case AccCondOpen:	close = AccClose; break;

			case BrackCondOpen: close = BrackClose; break;
			case BrackImportantOpen: close = BrackClose; break;
			};

			int res = seek(str, pos + 1, Lexem(close));
			if (res == -1) {
				return mkSeq(new AstNode(AstNodeType::Sentence, restringOne(str[pos])), parse(str, pos + 1));
			}

			vector<Lexem> sub(str.begin() + pos + 1, str.begin() + res);
			string strSub = Lex::lexemsToString(sub);
			AstNode * nEv = nullptr;
			switch (l.type) {
				default:throw "assert";
				case BrackOpen: 			nEv = new AstNode(AstNodeType::Event, strSub); break;
				case AccOpen: 				nEv = new AstNode(AstNodeType::UniqueEvent, strSub); break;
				case AccCondOpen: 			nEv = new AstNode(AstNodeType::CondUniqueEvent, strSub); break;
				case BrackCondOpen:			nEv = new AstNode(AstNodeType::CondEvent, strSub); break;
				case BrackImportantOpen:	nEv = new AstNode(AstNodeType::ImportantEvent, strSub); break;
			}
			return mkSeq( nEv, parse(str, res + 1));
		}
		break;
	}
	
	return nullptr;//unlikely to come here
}
