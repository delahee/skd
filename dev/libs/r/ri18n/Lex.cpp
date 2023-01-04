#include "stdafx.h"

#include "../utf8/utf8.h"

#include "Lex.hpp"

using namespace std;
using namespace ri18n;

static char SPECIAL_CHARS[] = { '|','[',']','{','}','<','>','*',':' };

ri18n::Lex::Lex(){
}

bool Lex::isSpecialChar(int cc){
	int sza = sizeof(SPECIAL_CHARS);
	for (int i = 0; i < sizeof(SPECIAL_CHARS); ++i) {
		if (SPECIAL_CHARS[i] == cc)
			return true;
	}
	return false;
}

string Lex::fromCharCode(int code) {
	char cc[4] = {};
	utf8catcodepoint(cc, code, 4);
	return string(cc);
}

string Lex::lexemToString( const Lexem & l) {
	switch (l.type) {
		default:					return string("ERR");

		case Char:					return fromCharCode(l.cc);//use new utf8.add?
		case Pipe:					return string("|");
		case Literal:				return l.strData;
		case AccOpen:				return string("}");
		case AccCondOpen:			return string("{?");
		case AccClose:				return string("}");
		case TagOpen:				return string("<") + l.strData + string(">");
		case TagClose:				return string("</") + l.strData + string(">");
		case TagSelfClosed:			return string("<") + l.strData + string("/>");
		case BrackOpen:				return string("[");
		case BrackCondOpen:			return string("[?");
		case BrackImportantOpen:	return string("[!");
		case BrackClose:			return string("]");
		case Star:					return string("*");
		case DoubleStar:			return string("**");
		case DoubleSemiColon:		return string("::");
		case BrackPosOpen:			return string("[>");
	}
}

string Lex::lexemsToString(const vector<Lexem> & l)
{
	string res;
	for (auto p : l) 
		res = res + lexemToString(p);
	return res;
}

string Lex::dumpLexems(const vector<Lexem>& l)
{
	string res;
	for (int i = 0; i < l.size(); i++) {
		switch (l[i].type) {
		case Char:						res.append("Char(" + fromCharCode(l[i].cc) + ") ");	 break;//use new utf8.add?
			case Pipe:					res.append("Pipe ");	 break;
			case Literal:				res.append("Literal("+l[i].strData+") ");	 break;
			case AccOpen:				res.append("AccOpen ");	 break;
			case AccCondOpen:			res.append("AccCondOpen ");	 break;
			case AccClose:				res.append("AccClose ");	 break;
			case TagOpen:				res.append("TagOpen(" + l[i].strData + ") ");	 break;
			case TagClose:				res.append("TagClose(" + l[i].strData + ") ");	 break;
			case TagSelfClosed:			res.append("TagSelfClosed(" + l[i].strData + ") ");	 break;
			case BrackOpen:				res.append("BrackOpen ");	 break;
			case BrackCondOpen:			res.append("BrackCondOpen ");	 break;
			case BrackImportantOpen:	res.append("BrackImportantOpen ");	 break;
			case BrackClose:			res.append("BrackClose ");	 break;
			case Star:					res.append("Star ");	 break;
			case DoubleStar:			res.append("DoubleStar ");	 break;
			case DoubleSemiColon:		res.append("DoubleSemiColon ");	 break;
			case BrackPosOpen:			res.append("BrackPosOpen ");	 break;
		}
	}
	return res;
}

bool Lex::isSentenceChar(int cc){
	return !isSpecialChar(cc);
}

vector<Lexem> Lex::parse(const string& str) {
	return parse(str.c_str());
}

vector<Lexem> Lex::parse(const char * str){
	cur = str;
	vector<Lexem> res;
	vector<int>codes;
	const char * cstr = str;
	while (*cstr != 0) {
		int cp = 0;
		cstr = (char*) utf8codepoint(cstr, &cp);
		codes.push_back(cp);
	}
	_parse(codes, 0, res);
	return res;
}
/*
int firstChar(const char * cstr) {
	int res = 0;
	utf8codepoint(cstr, &res);
	return res;
}
*/

int c32Len(const vector<int> & codes) {
	for (int i = 0; i < codes.size(); ++i) 
		if (codes[i] == 0)
			return i;
	return codes.size();
}

vector<int>c32Sub(const vector<int> & codes, int pos, int len) {
	vector<int> res;
	for (int i = pos; i < pos + len; ++i)
		res.push_back(codes[i]);
	return res;
}

string getUtf8fromC32(int code) {
	string res;
	char utf8[4] = {};
	int sz = utf8codepointsize(code);
	utf8catcodepoint(utf8, code, 4);
	for (int i = 0; i < sz; i++)
		res.push_back(utf8[i]);
	return res;
}

string c32toUtf8(const vector<int> & codes) {
	string res;
	int size = 0;
	char utf8[4] = {};
	for (int i = 0; i < codes.size(); ++i) {
		int code = codes[i];
		res.append(getUtf8fromC32(code));
	}
	return res;
}

int charCodeAt(const vector<int> & str, int pos) {
	return str[pos];
};

void Lex::_parse(const vector<int> & str, int pos, vector<Lexem>& result){
	//if ( DEBUG ) trace( "cur res stack:" + result + " upon " + str + " pos:" + pos );
	int len = c32Len(str);
	if (pos >= len)
		return;

	switch (str[pos]) {
		case ':':
		{
			int pp = pos + 1;
			if ((pp < len) && (str[pp] == ':')) {
				result.push_back(Lexem(DoubleSemiColon));

				pp++;
				int opp = pp;
				while (pp < len-1 && str[pp] != ':' && str[pp + 1] != ':')
					pp++;

				if (pp >= len-1) {
#ifdef _DEBUG
					cerr << "ri8n::WARNING::suspicious double semi colon " << c32toUtf8(str) << endl;
#endif
					vector<int> sub(str.begin() + opp, str.end());
					string lit = c32toUtf8(sub);
					result.push_back(Lexem(Literal, lit));
					result.push_back(Lexem(DoubleSemiColon));
				}
				else {
					int endpp = pp + 1;
					int subLen = endpp - opp;
					string lit = c32toUtf8(c32Sub(str, opp, subLen));
					result.push_back(Lexem(Literal, lit));
					result.push_back(Lexem(DoubleSemiColon));
					_parse(str, pp + 3, result);
				}
			}
			else {
				result.push_back(Lexem(Char, ':'));
				_parse(str, pos + 1, result);
			}
			break;
		}
		case '\\':
		if ((pos + 1 < len)
			&& isSpecialChar(charCodeAt(str, pos + 1))) {
			result.push_back(Lexem(Char,charCodeAt(str, pos + 1)));
			_parse(str, pos + 2, result);
		}
		else {
			result.push_back(Lexem(Char,'\\'));
			_parse(str, pos + 1, result);
		}
		break;

		case '|':{
			int start = pos;
			int end = pos + 1;
			while (end < len && charCodeAt(str, end) != '|') {
				end++;
			}
			if (end == len) {
				result.push_back(Lexem(Char,'|'));
			}
			else {
				int subLen = end - (start + 1);
				string sub(str.begin() + start + 1, str.begin() + start + 1+subLen);
				result.push_back(Lexem(Pipe));
				result.push_back(Lexem(Literal,sub));
				result.push_back(Lexem(Pipe));
				_parse(str, end + 1, result);
			}
		}break;
		case '{': 
		{
			int pp = pos + 1;
			if ((pos + 1 < len)
				&& str[pp] == '?') {
				result.push_back(Lexem(AccCondOpen));
				pp++;
			}
			else
				result.push_back(Lexem(AccOpen));
			_parse(str, pp, result);
			break;
		}

		case '[': {
			int pp = pos + 1;
			int len = str.size();

			if ((pos + 1 < len)
				&& str[pp] == '>')
			{
				result.push_back(Lexem(BrackPosOpen));
				pp++;
			}
			else
				if ((pos + 1 < len)
					&& str[pp] == '?') {
					result.push_back(Lexem(BrackCondOpen));
					pp++;
				} 
				else if ((pos + 1 < len)
					&& str[pp] == '!') {
					result.push_back(Lexem(BrackImportantOpen));
					pp++;
				}
				else
					result.push_back(Lexem(BrackOpen));
			_parse(str, pp, result);
			break;
		}
		
		
		case '>'://tag close should be detected out much sooner so it's a dangling char
		{
			result.push_back(Lexem(Char, '>'));
			_parse(str, pos + 1, result);
		}
		break;
		
		case '<':
		{
			int pp = pos + 1;
			if ((pp < len)
				&& str[pp] == '/') {
				pp++;
				while (pp < len && charCodeAt(str, pp) != '>')
					pp++;

				if (pp == len) { //not found cancel
					result.push_back(Lexem(Char, '<'));
					_parse(str, pos + 1, result);
					return;
				}
				else {
					int sublen = pp - (pos + 2);
					vector<int> sub(str.begin() + pos + 2, str.begin() + pos + 2 + sublen);
					string lit = string(c32toUtf8(sub));
					result.push_back(Lexem(TagClose, lit));
					_parse(str, pp + 1, result);
				}
			}
			else {
				while (pp < len && charCodeAt(str, pp) != '>')
					pp++;

				if (pp == len) { //not found cancel
					result.push_back(Lexem(Char, '<'));
					_parse(str, pos + 1, result);
					return;
				}
				else {

					if (charCodeAt(str, pp - 1) == '/') {
						int subLen = (pp - 1) - (pos + 1);
						auto first = str.begin() + pos + 1;
						auto last = first + subLen;

						while (rd::String::isWhitespace(*first) && (first != str.end()))
							first++;

						while (rd::String::isWhitespace(*(last-1)) && (last != str.begin()))
							last--;

						vector<int> v(first, last);
						string lit = c32toUtf8(v);
						result.push_back(Lexem(TagSelfClosed, lit));
						_parse(str, pp + 1, result);
					}
					else {
						int subLen = pp - (pos + 1);

						auto first = str.begin() + pos + 1;
						auto last = first + subLen;
						vector<int> v(first, last);

						string lit = c32toUtf8(v);
						result.push_back(Lexem(TagOpen, lit));
						_parse(str, pp + 1, result);
					}
				}
			}
		}
		break;

		case '}':
			result.push_back(Lexem(AccClose));
			_parse(str, pos + 1, result);
			break;

		case ']':
			result.push_back(Lexem(BrackClose));
			_parse(str, pos + 1, result);
			break;
		case '*': {
			if (	(pos < str.size()-1)
				&&	(charCodeAt(str, pos + 1) == '*')) {
				result.push_back(Lexem(DoubleStar));
				pos++;
			}
			else {
				result.push_back(Lexem(Star));
			}
			_parse(str, pos+1, result);
		}
		break;
		default:
		{
			int c = charCodeAt(str, pos);
			while (isSentenceChar(c)) {
				result.push_back(Lexem(Char,c));
				pos++;
				if (pos >= str.size()) {
					return;
				}
				c = charCodeAt(str, pos);
			}	
			if(!isSentenceChar(c)){
				_parse(str, pos, result);
			}
			else {
#ifdef _DEBUG
				cout << "WTF"<< endl;
#endif
			}
		}
		break;
	}
}
