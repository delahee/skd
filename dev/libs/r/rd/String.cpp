#include "stdafx.h"

#include <codecvt>
#include <cctype>
#include <iostream>
#include <string>

#include "../utf8/utf8.h"

#include "String.hpp"
#include "UTF8Utils.hpp"
#include "Vars.hpp"

int rd::String::strlenUTF8(const char * input) {
	int len = 0;
	// Add length if char != 10xxxxxx
	// https://fr.wikipedia.org/wiki/UTF-8#Description
	while (*input) {
		if (( *input & 0b11000000 ) != 0b10000000)
			len++;
		*input++;
	}

	return len;
}

int rd::String::strlenUTF8(const std::string & input) {
	if (input.empty())
		return 0;
	int len = 0;
	// Add length if char != 10xxxxxx
	// https://fr.wikipedia.org/wiki/UTF-8#Description
	for (int i = 0; i < input.size(); i++) {
		if (input[i] == 0)
			break;
		if (( input[i] & 0b11000000 ) != 0b10000000)
			len++;
	}

	return len;
}

std::string rd::String::toString(const std::wstring & wstr)
{
	std::wstring_convert<std::codecvt_utf8<wchar_t>> myconv;
	std::string str = myconv.to_bytes(wstr);
	return str;
}

std::wstring rd::String::toWString(const std::string & str)
{
	const UTF8Char * u8Str = (const UTF8Char *)str.c_str();
	int len = 1 + rd::UTF8Utils::StrLen((const UTF8Char *) str.c_str());
	std::wstring wstr;
	wstr.resize(len);
	rd::UTF8Utils::utf8ToWchar(wstr.data(), u8Str, len);
	return wstr;
}

const char* rd::String::skip(const char* forOpen, const char* toSkip) {
	return strstr(forOpen, toSkip) + strlen(toSkip);
}

const char* rd::String::skip(const std::string &forOpen, const char* toSkip){
	return strstr(forOpen.c_str(), toSkip) + strlen(toSkip);
}

std::vector<std::string> rd::String::split(const std::string& s, char delimiter){
	if (s.empty()) return {};

	size_t pos_start = 0, pos_end;
	std::string token;
	std::vector<std::string> res;
	while ((pos_end = s.find(delimiter, pos_start)) != std::string::npos) {
		token = s.substr(pos_start, pos_end - pos_start);
		pos_start = pos_end + 1;
		res.push_back(token);
	}
	res.push_back(s.substr(pos_start));
	return res;
}

std::vector<std::string> rd::String::split(const std::string &s, const std::string & delimiter) {
	size_t pos_start = 0, pos_end, delim_len = delimiter.length();
	std::string token;
	std::vector<std::string> res;

	while ((pos_end = s.find(delimiter, pos_start)) != std::string::npos) {
		token = s.substr(pos_start, pos_end - pos_start);
		pos_start = pos_end + delim_len;
		res.push_back(token);
	}

	res.push_back(s.substr(pos_start));
	return res;
}

std::string rd::String::join(const eastl::vector<std::string>& s, const std::string& delimiter) {
	std::string res;

	//wild guess to limit allocs a bit
	int len = 0;
	for (auto& str : s)
		len += str.size();
	res.reserve(len + s.size() * delimiter.size());

	int idx = 0;
	for (auto& str : s) {
		if (idx == s.size() - 1)
			res = res + str;
		else
			res = res + str + delimiter;
		idx++;
	}

	return res;
}

std::string rd::String::join(const std::vector<std::string>& s, const std::string& delimiter) {
	std::string res;

	//wild guess to limit allocs a bit
	int len = 0;
	for (auto& str : s)
		len += str.size();
	res.reserve(len + s.size() * delimiter.size());

	int idx = 0;
	for (auto& str : s) {
		if (idx == s.size() - 1)
			res = res + str;
		else 
			res = res + str + delimiter;
		idx++;
	}

	return res;
}

Str rd::String::join(const std::vector<Str>& s, const Str& delimiter) {
	Str res;

	//wild guess to limit allocs a bit
	int len = 0;
	for (auto& str : s)
		len += str.length();
	res.reserve(len + s.size() * delimiter.length());

	int idx = 0;
	for (auto& str : s) {
		if (idx == s.size() - 1)
			res = res + str;
		else
			res = res + str + delimiter;
		idx++;
	}
	return res;
}

Str rd::String::join(const eastl::vector<Str>& s, const Str& delimiter) {
	Str res;

	//wild guess to limit allocs a bit
	int len = 0;
	for (auto& str : s)
		len += str.length();
	res.reserve(len + s.size() * delimiter.length());

	int idx = 0;
	for (auto& str : s) {
		if (idx == s.size() - 1)
			res = res + str;
		else
			res = res + str + delimiter;
		idx++;
	}
	return res;
}

std::string rd::String::replace(const std::string& original, const std::string& pattern, const std::string& newPattern) {
	if (original.find(pattern) == std::string::npos)
		return original;
	std::vector<std::string> vec = split(original, pattern);
	std::string res;
	int i = 0;
	for (std::string& val : vec) {
		res.append(val);
		if (i != vec.size() - 1)
			res.append(newPattern);
		i++;
	}
	return res;
}

Str rd::String::replace(const Str& original, const Str& pattern, const Str& newPattern){
	if ( !strstr(original.c_str(),pattern.c_str()) )
		return original;
	std::vector<std::string> vec = split(original.cpp_str(), pattern.cpp_str());
	std::string res;
	int i = 0;
	for (std::string& val : vec) {
		res.append(val);
		if (i != vec.size() - 1)
			res.append(newPattern.cpp_str());
		i++;
	}
	return Str(res);
}

bool rd::String::getLastNumber(const char* str, int & _val){
	const char* end = str + strlen(str)-1;
	while (end >= str && (*end>='0'&&*end<='9')) {
		end--;
	}
	end++;
	if (end <= str - 1) 
		return false;
	int val = 0;
	int nb = sscanf(end, "%d", &val);
	if (nb>=1) {
		_val = val;
		return true;
	}
	return false;
}


char* rd::String::stristr(char* str, const char* p) {
	return const_cast<char*>(stristr((const char *) str, p));
}

//https://opensource.apple.com/source/CyrusIMAP/CyrusIMAP-156.8/cyrus_imap/lib/stristr.c.auto.html
const char* rd::String::stristr(const char* String, const char* Pattern) {
	char* pptr, * sptr, * start;
	size_t  slen, plen;

	for (start = (char*)String,
		pptr = (char*)Pattern,
		slen = strlen(String),
		plen = strlen(Pattern);

		/* while string length not shorter than pattern length */

		slen >= plen;

		start++, slen--)
	{
		/* find start of pattern in string */
		while (toupper(*start) != toupper(*Pattern))
		{
			start++;
			slen--;

			/* if pattern longer than string */

			if (slen < plen)
				return(NULL);
		}

		sptr = start;
		pptr = (char*)Pattern;

		while (toupper(*sptr) == toupper(*pptr))
		{
			sptr++;
			pptr++;

			/* if end of pattern then pattern was found */

			if ('\0' == *pptr)
				return (start);
		}
	}
	return nullptr;
}

bool rd::String::startsWithI(const char* String, const char* pattern){
	return strncasecmp(String, pattern,strlen(pattern))==0;
}

bool rd::String::startsWithI(const Str& str, const char* pattern) {
	return strncasecmp(str.c_str(), pattern, strlen(pattern)) == 0;
}

bool rd::String::startsWith(const char* String, const char* Pattern)
{
	return strncmp(String,Pattern,strlen(Pattern))==0;
}

bool rd::String::startsWith(const std::string& str, const char * pat){
	return startsWith(str.c_str(),pat);
}

bool rd::String::contains(const std::string& str, const char* Pattern) {
	return strstr(str.c_str(), Pattern) != nullptr;
}

bool rd::String::contains(const char* str, const char* Pattern) {
	return strstr(str, Pattern) != nullptr;
}

const char* strstrni(const char* str, const char* p, int nb) {
	while (str && p && *str && *p) {
		if (0 == strncasecmp(str, p, nb))
			return str;
		str++;
	}
	return nullptr;
}

const char* strstri(const char* str, const char* p) {
	return strstrni(str, p, strlen(p));
}

bool rd::String::containsI(const char* str, const char* p){
	return strstri(str, p) != nullptr;
}

bool rd::String::equals(const std::string& str0, const char* str1) {
	return 0 == strcmp(str0.c_str(), str1);
}

bool rd::String::equals(const Str& str0, const char* str1) {
	return str0 == str1;
}

bool rd::String::equals(const char* str0, const char* str1) {
	return 0==strcmp(str0, str1);
}

bool rd::String::equalsI(const std::string& str0, const char* str1) {
	return 0 == strcasecmp(str0.c_str(), str1);
}

bool rd::String::equalsI(const char* str0, const char* str1) {
	return 0 == strcasecmp(str0, str1);
}

bool rd::String::equalsI(const Str& str0, const char* str1){
	return 0 == strcasecmp(str0.c_str(), str1);
}

enum STATE
{
	ParseKey,
	ParseVal,
};

bool rd::String::isWhitespace(int cdpt) {
	return (cdpt == ' ') || (cdpt == '\r') || (cdpt == '\t') || (cdpt == '\n');
}

int * rd::String::skipWhitespace(int * cdpts) {
	while (*cdpts && isWhitespace(*cdpts) )
		cdpts++;
	return cdpts;
}

const char * rd::String::skipWhitespace(const char * cdpts) {
	if (!cdpts) return cdpts;
	while (*cdpts && isWhitespace(*cdpts))
		cdpts++;
	return cdpts;
}

std::string rd::String::c32toUtf8(int * codes, int forceLen ) {
	std::string res;
	int size = 0;
	char utf8[4] = {};
	int len = (forceLen!=-1) ? forceLen : c32Len(codes);
	for (int i = 0; i < len; ++i) {
		int code = codes[i];
		res.append(getUtf8fromC32(code));
	}
	return res;
}

std::string rd::String::getUtf8fromC32(int code) {
	std::string res;
	char utf8[4] = {};
	int sz = utf8codepointsize(code);
	utf8catcodepoint(utf8, code, 4);
	for (int i = 0; i < sz; i++)
		res.push_back(utf8[i]);
	return res;
}

std::string& rd::String::toLower(std::string&str){
	std::transform(str.begin(), str.end(), str.begin(),
		[](unsigned char c) { 
			return std::tolower(c); 
		});
	return str;
}

std::string& rd::String::toUpper(std::string& str) {
	std::transform(str.begin(), str.end(), str.begin(),
		[](unsigned char c) {
			return std::tolower(c);
		});
	return str;
}

int rd::String::c32Len(int * codes) {
	int* start = codes;
	while (*codes) 
		codes++;
	return codes - start;
}

void rd::String::parseXMLTags(const char* tag, XmlData& res){
	std::vector<std::string> spaced = split(tag, " ");
	res.tag = spaced[0];
	spaced.erase(spaced.begin());
	std::string rest = join(spaced, " ");
	std::vector<int> toCp;
	toCodePoints(rest.c_str(), toCp);
	toCp.push_back(0);
	int* cur = skipWhitespace( toCp.data() );
	STATE st = ParseKey;
	int* head = toCp.data();
	int* keyStart = cur;
	int* keyEnd = cur;

	int* literalStart = cur;
	int* literalEnd = cur;

	for (;;) {
		int pos = cur - head;
		if (pos >= toCp.size()) 
			break;
		if (!cur)
			break;

		if (st == ParseKey) {
			if (*cur != '=') {
				cur++;
			}
			else {
				//we are on '='
				keyEnd = cur;
				cur++;
				if (*cur == '>') {
					//finished
					std::string k = c32toUtf8(keyStart, keyEnd - keyStart);
					res.attrs[k] = "";
					break;
				}
				while(isWhitespace(*cur)) 
					cur++;
				if (*cur == '"' || *cur == '\'') {
					cur++;
					literalStart = cur;
					st = ParseVal;
				}
			}
		}
		else{
			if (!cur) {
				break;
			}
			if (*cur == '"' || *cur == '\'') {
				literalEnd = cur;
				cur++;
				std::string k = c32toUtf8(keyStart, keyEnd- keyStart);
				std::string v = c32toUtf8(literalStart, literalEnd - literalStart);
				res.attrs[k] = v;
				st = ParseKey;

				while (isWhitespace(*cur)) {
					cur++;
					literalStart = cur;
					keyStart = cur;
				}
			}
			else {
				cur++;
			}
		}
	}
	int done = 0;
}

void rd::String::toCodePoints(const char* utf8String, std::vector<int>  & res)
{
	res.clear();
	const char* cstr = utf8String;
	while (*cstr != 0) {
		int cp = 0;
		cstr = (char*)utf8codepoint(cstr, &cp);
		res.push_back(cp);
	}
}

Str rd::String::parseIdentifier(const char* str){
	std::string res;
	const char* sstr = str;
	while (*str && !isWhitespace(*str) && (*str != ',') && (*str != ')') && (*str != ' '))
		str++;
	res.assign(sstr, str - sstr);
	return res;
}

std::vector<Str> rd::String::parseIdentifierList(const char* lit) {
	std::vector<Str> res;
	const char* str = lit;
	str = rd::String::skipWhitespace(str);
	do {
		Str one = rd::String::parseIdentifier(str);
		if (one.length() == 0)
			return res;
		res.push_back(one);
		str += one.length();
		while (*str && ((*str == ',') || (*str == ' ') || rd::String::isWhitespace(*str)))
			str++;
	} while (*str);
	return res;
}

const char* rd::String::skipToParenOrComma(const char * src) {
	while (*src && (*src != ',') && (*src != ')'))
		src++;
	return src;
}

bool rd::String::isInteger(const char* str) {
	int v = 0;
	double f = 0;
	str = rd::String::skipWhitespace(str);
	bool readInt = sscanf(str, "%d", &v);
	bool readFloat = sscanf(str, "%lf", &f);

	if (!readInt)
		return false;

	if (readInt && !readFloat)
		return false;

	return Math::approximatelyEqual((double)v, f);
}

bool rd::String::isFloat(const char* str){
	double f = 0;
	str = rd::String::skipWhitespace(str);
	bool readFloat = sscanf(str, "%lf", &f);
	if (!readFloat)
		return false;
	return f;
}

bool rd::String::parseBool(const char* str){
	if (0 == strcasecmp(str, "false"))
		return false;
	if (0 == strcasecmp(str, "0"))
		return false;
	if (0 == strcasecmp(str, "faux"))
		return false;

	if (0 == strcasecmp(str, "true"))
		return true;
	if (0 == strcasecmp(str, "vrai"))
		return true;
	if (0 == strcasecmp(str, "1"))
		return true;

	return false;
}

bool rd::String::isBoolean(const char* str){
	if (0 == strcasecmp(str, "false"))
		return true;
	if (0 == strcasecmp(str, "0"))
		return true;
	if (0 == strcasecmp(str, "faux"))
		return true;

	if (0 == strcasecmp(str, "true"))
		return true;
	if (0 == strcasecmp(str, "vrai"))
		return true;
	if (0 == strcasecmp(str, "1"))
		return true;

	return false;
}

bool rd::String::isAlphaNum(int cdpt){
	if (cdpt >= 'a' && cdpt <= 'z')
		return true;
	if (cdpt >= 'A' && cdpt <= 'Z')
		return true;
	if (cdpt >= '0' && cdpt <= '9')
		return true;
	return false;
}

bool rd::String::isIdentifierCharacter(int cdpt){
	return isAlphaNum(cdpt) ||cdpt == '_';
}

r::Color rd::String::readColor(const char* lit) {
	bool readHexa = false;
	if (startsWith(lit, "0x")) {
		lit += 2;
		readHexa = true;
	}
	if (startsWith(lit, "#")) {
		lit ++;
		readHexa = true;
	}
	if(readHexa){
		unsigned int color = 0;
		int nLen = 0;
		const char* litNum = lit;
		while (*litNum  && (*litNum >= '0') && (*litNum <= 'f')) {
			litNum++;
			nLen++;
		}
		auto ret = sscanf(lit, "%x", &color);
		if(nLen == 6 )
			return r::Color::fromUInt24(color);
		if (nLen == 8)
			return r::Color::fromUInt(color);
		else
			return r::Color();
	}
	return r::Color();
}

std::string rd::String::trim(const std::string& str){
	std::string s;
	int len = str.length();
	int i = 0;
	for (; i < len; ) {
		if (isWhitespace(str[i]))
			i++;
		else 
			break;
	}

	int end = str.length()-1;
	while (isWhitespace(str[end]) && (end > 0))
		end--;

	s = str.substr(i, end - i + 1);
	return s;
}

void rd::String::rtrimInPlace(Str& s){
	int sz = s.length();
	for (int i = 0; i < sz; ++i) {
		int endi = sz - i - 1;
		char* point = s.c_str() + endi;
		if (*point == ' ')
			*point = 0;
		else
			return;
	}
}

bool rd::String::endsWith(const char* str, const char* suffix) {
	int strL = strlen(str);
	int suffL = strlen(suffix);
	if (suffL >= strL) return false;

	return strcmp(str + strL - suffL, suffix) == 0;
}

void rd::String::replace(char* str, char tok, char repl){
	if (!str) return;
	if (!*str) return;
	int sz = strlen(str);
	while(*str){
		if(*str == tok)
			*str = repl;
		str++;
	}
}

bool rd::String::hasTag(const char* str, const char* tag){
	return rd::String::containsI(str, tag);
}