#include "stdafx.h"

#include <codecvt>
#include <cctype>
#include <iostream>
#include <string>

#include "../utf8/utf8.h"

#include "String.hpp"
#include "UTF8Utils.hpp"
#include "Vars.hpp"

std::string rd::String::empty;
Str rd::String::emptyStr;

const char* strrstr(const char* haystack, const char* needle) {
	const char* r = 0;
	if (!needle[0])
		return haystack + strlen(haystack);
	while (true) {
		const char* p = strstr(haystack, needle);
		if (!p)
			return r;
		r = p;
		haystack = p + 1;
	}
};

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
	if (delimiter.empty()) return {};
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

eastl::vector<Str> rd::String::splitStr(const char* s, const char* delim){
	if (!delim || !*delim) return {};
	eastl::vector<Str> res;
	const char* head = s;
	int delimLen = strlen(delim);
	while(head && *head) {
		auto oh = head;
		head = strstr(head, delim);
		res.push_back(rd::String::sub(oh, head));
		if (head)
			head += delimLen;
		else
			break;
	}
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

std::string rd::String::join(const char** s, int nb, const std::string& delimiter){
	std::string res;

	//wild guess to limit allocs a bit
	int len = 0;
	for (int i =0; i < nb; ++i)
		len += strlen(s[i]);
	res.reserve(len + nb * delimiter.size());
	int idx = 0;
	for (int i = 0; i < nb; ++i){
		const char* str = s[i];
		if (idx == (nb-1))
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

/*
std::string rd::String::replace(const std::string& original, const char* pattern, const char* newPattern) {
	if (original.find(pattern) == std::string::npos)
		return original;
	std::vector<std::string> vec = rd::String::split(original, pattern);
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

Str rd::String::replace(const Str& original, const char* pattern, const char* newPattern) {
	if (!rd::String::contains(original.c_str(), pattern))
		return original;
	std::vector<std::string> vec = split(original.cpp_str(), pattern);
	std::string res;
	int i = 0;
	for (std::string& val : vec) {
		res.append(val);
		if (i != vec.size() - 1)
			res.append(newPattern);
		i++;
	}
	return Str(res);
}
*/

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

int rd::String::find(const Str& s, const char* pattern){
	const char* f = strstr(s.c_str(), pattern);
	if (!f) return -1;
	return f - s.c_str();
}

int rd::String::rfind(const Str& s, const char* pattern) {
	const char* f = strrstr(s.c_str(), pattern);
	if (!f) return -1;
	return f - s.c_str();
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

bool rd::String::contains(const char* string, char pattern){
	return strchr(string,pattern);
}

bool rd::String::contains(const Str& str, char pattern) { 
	return contains(str.c_str(), pattern); 
}

bool rd::String::contains(const Str& str, const char* pattern) {
	return strstr(str.c_str(), pattern) != nullptr;
}

bool rd::String::contains(const char* str, const char* pattern) {
	return strstr(str, pattern) != nullptr;
}

static const char* strstrni(const char* str, const char* p, int nb) {
	while (str && p && *str && *p) {
		if (0 == strncasecmp(str, p, nb))
			return str;
		str++;
	}
	return nullptr;
}

static const char* strstri(const char* str, const char* p) {
	if (str == 0) return false;//null string are equal to nobody
	if (p == 0) return false;//null string are equal to nobody
	return strstrni(str, p, strlen(p));
}

bool rd::String::containsI(const char* str, const char* p){
	if (str == 0) return false;//null string are equal to nobody
	if (p == 0) return false;//null string are equal to nobody
	return strstri(str, p) != nullptr;
}

bool rd::String::equals(const std::string& str0, const char* str1) {
	if (str1 == 0) return false;//null string are equal to nobody
	return 0 == strcmp(str0.c_str(), str1);
}

bool rd::String::equals(const Str& str0, const char* str1) {
	if (str1 == 0) return false;//null string are equal to nobody
	return str0 == str1;
}

bool rd::String::equals(const char* str0, const char* str1) {
	if (str0 == 0) return false;//null string are equal to nobody
	if (str1 == 0) return false;//null string are equal to nobody
	return 0==strcmp(str0, str1);
}

bool rd::String::equalsI(const std::string& str0, const char* str1) {
	if (str1 == 0) return false;//null string are equal to nobody
	return 0 == strcasecmp(str0.c_str(), str1);
}

bool rd::String::equalsI(const char* str0, const char* str1) {
	if (str0 == 0) return false;//null string are equal to nobody
	if (str1 == 0) return false;//null string are equal to nobody
	return 0 == strcasecmp(str0, str1);
}

bool rd::String::equalsI(const Str& str0, const char* str1){
	return 0 == strcasecmp(str0.c_str(), str1);
}

bool rd::String::equalsI(const Str& str0, const Str& str1) {
	return 0 == strcasecmp(str0.c_str(), str1.c_str());
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

int rd::String::utf8code(const char* str){
	if (!str|| !*str)return 0;
	int codes[2] = {0,0};
	utf8codepoint(str, codes);
	return codes[0];
}

eastl::vector<int> rd::String::utf8ToC32(const std::string& str){
	eastl::vector<int> res;
	toCodePoints(str.c_str(), res);
	return res;
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

bool rd::String::isLower(std::string& str){
	for (auto& c : str)
		if (c != std::tolower(c))
			return false;
	return true;
}

std::string& rd::String::toLower(std::string&str){
	std::transform(str.begin(), str.end(), str.begin(),
		[](unsigned char c) { 
			return std::tolower(c); 
		});
	return str;
}

void rd::String::lower(char * str, int count){
	for (int i = 0; i < count; ++i) 
		str[i] = std::tolower(str[i]);
}

void rd::String::upper(char* str, int count){
	for (int i = 0; i < count; ++i)
		str[i] = std::toupper(str[i]);
}

std::string& rd::String::toUpper(std::string& str) {
	std::transform(str.begin(), str.end(), str.begin(),
		[](unsigned char c) {
			return std::toupper(c);
	});
	return str;
}

std::string& rd::String::capitalize(std::string& str) {
	int idx = 0;
	std::transform(str.begin(), str.end(), str.begin(),
		[&](unsigned char c) {
			auto oc = 0;
			if (idx == 0)
				oc = std::toupper(c);
			else
				oc = std::tolower(c);
			idx++;
			return oc;
		});
	return str;
}

std::string rd::String::toCapitalized(const std::string & _str) {
	std::string str = _str;
	capitalize(str);
	return str;
}

std::string& rd::String::capitalizeFirstChar(std::string& str) {
	if(!str.empty())
		str[0] = std::toupper(str[0]);

	return str;
}

std::string& rd::String::camelCase(std::string& str) {
	int len = str.length();
	for (int i = 0; i < len; ++i) {
		char& c = str[i];
		char oc = c;
		bool apply = (i == 0);
		if (i > 0)
			apply = str[i - 1] == ' ';
		if (apply)
			c = std::toupper(c);
		else
			c = std::tolower(c);
	}
	return str;
}

std::string rd::String::toCamelCase(const std::string& _str){
	std::string str = _str;
	camelCase(str);
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

void rd::String::toCodePoints(const char* utf8String, std::vector<int>  & res){
	res.clear();
	const char* cstr = utf8String;
	while (*cstr != 0) {
		int cp = 0;
		cstr = (char*)utf8codepoint(cstr, &cp);
		res.push_back(cp);
	}
}

void rd::String::toCodePoints(const char* utf8String, eastl::vector<int>  & res){
	res.clear();
	const char* cstr = utf8String;
	while (*cstr != 0) {
		int cp = 0;
		cstr = (char*)utf8codepoint(cstr, &cp);
		res.push_back(cp);
	}
}

Str rd::String::parseIdentifier(const char* str){
	if (!str || !*str) return {};

	std::string res;
	const char* sstr = str;
	while (*str && !isWhitespace(*str)) {
		str++;
		if (*str == ',')
			break;
		if (*str == '(')
			break;
		if (*str == ')')
			break;
		if (*str == '<')
			break;
		if (*str == '>')
			break;
		if (*str == '=')
			break;
		if (*str == '!')
			break;
	}
	res.assign(sstr, str - sstr);
	return res;
}

eastl::vector<int> rd::String::parseIntList(const char* str) {
	eastl::vector<int> l;
	int c = 0;
	int ret = 0;
	while(str&&*str&& (ret = sscanf(str, "%d ", &c))) {
		str = strstr(str, " ");
		l.push_back(c);
		if (!str||!*str) break;
		str++;
	}
	return l;
}

eastl::vector<int> rd::String::parseIntList(const char* lit, const char* sep){
	if (!lit || !*lit) return {};
	const char* str = lit;
	str = rd::String::skipWhitespace(str);

	if (!str || !*str) return {};

	Str splFmt = StrRef("%d")+ sep;
	int patternLen = strlen(sep);
	eastl::vector<int> l;
	int c = 0;
	int ret = 0;
	while (str && *str && (ret = sscanf(str, splFmt.c_str(), &c))) {
		str = strstr(str, sep);
		l.push_back(c);
		if (!str || !*str) break;
		str += patternLen;
	}
	return l;
}

eastl::vector<Str> rd::String::parseIdentifierList(const char* lit) {
	eastl::vector<Str> res;
	const char* str = lit;

	if (!str || !*str) return {};

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

const char* rd::String::skipTo(const char* forOpen, int code) {
	return strchr(forOpen, code);
}

const char* rd::String::skipTo(const char* forOpen, const char* to){
	return strstr(forOpen, to);
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

int rd::String::parseInt(const char* str, int dflt){
	if (!str||!*str) return 0;
	int f = 0;
	str = rd::String::skipWhitespace(str);
	if (!str || !*str) return 0;
	bool read = sscanf(str, "%d", &f);
	if (!read)
		return dflt;
	return f;
}

int rd::String::parseInt(const char* str, int dflt, const char*& end){
	if (!str || !*str) return 0;
	 
	int f = 0;
	str = rd::String::skipWhitespace(str);
	if (!str || !*str) return 0;

	bool read = sscanf(str, "%d", &f);
	end = str;
	if (!read)
		return dflt;
	while (*str) {
		if (isNum(*str))
			str++;
		else {
			end = str;
			break;
		}
	}
	end = str;
	return f;
}

double rd::String::parseFloat(const char* str, double dflt){
	if (!str || !*str) return 0.0f;

	double f = 0;
	str = rd::String::skipWhitespace(str);
	bool readFloat = sscanf(str, "%lf", &f);
	if (!readFloat)
		return dflt;
	return f;
}

double rd::String::parseFloat(const char* str, double dflt, const char*& end){
	if (!str || !*str) return 0.0;

	double f = 0.0;
	str = rd::String::skipWhitespace(str);
	bool readFloat = sscanf(str, "%lf", &f);
	end = str;
	if (!readFloat)
		return dflt;
	while (*str) {
		if (isNumFloat(*str)) {
			if (*str && *str == '%') {
				f /= 100.0;
				break;
			}
			str++;
		}
		else {
			
			end = str;
			break;
		}
	}
	end = str;
	return f;
}

bool rd::String::isBoolean(const char* str){
	if (!str || !*str) return false;

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

bool rd::String::isNum(int cdpt){
	if (cdpt >= '0' && cdpt <= '9')
		return true;
	if (cdpt == '-')
		return true;
	return false;
}

bool rd::String::hasFloatSymbols(char * str) {

	while (str && *str) {
		if (*str == '.')
			return true;
		if (*str == '%')
			return true;
		str++;
	}
	return false;
}

bool rd::String::isNumFloat(int cdpt){
	if(isNum(cdpt))
		return true;
	if( cdpt == '.')
		return true;
	if (cdpt == '%')
		return true;

	return false;
}

static std::string s_forbiddenForIdents = "[](){}<>°'\"^$%µ*¨^!? \r\n=+-|`²¤$€";

bool rd::String::isIdentifierCharacterEmojiCompat(int cdpt) {
	for (auto c : s_forbiddenForIdents) 
		if (cdpt == c)
			return false;
	return true;
}

bool rd::String::isIdentifierCharacter(int cdpt){
	return isAlphaNum(cdpt) ||cdpt == '_';
}

bool rd::String::isEmpty(const char* str) {
	return (0==str) || (0 == *str);
}

bool rd::String::isColon(const char* str)
{
	return *str == ':';
}

bool rd::String::isNewline( int code ){
	return code == '\r' || code == '\n';
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

Str rd::String::sub(const char* start, const char* end) {
	Str s;
	s.append(start, end);
	return s;
}

const char* rd::String::ltrim(const char* s){
	while (s && *s && (*s == ' '))
		s++;
	return s;
}

std::string rd::String::trim(const std::string& str){
	if (str.empty())
		return str;
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

bool rd::String::endsWithI(const char* str, const char* suffix){
	int strL = strlen(str);
	int suffL = strlen(suffix);
	if (suffL >= strL) return false;
	return strcasecmp(str + strL - suffL, suffix) == 0;
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

bool rd::String::addTag(Str& str, const char* tag) {
	if (!tag) return false;
	if (!*tag) return false;

	if (str.empty())
		str = tag;
	else {
		str += ",";
		str += tag;
	}
	return true;
}

bool rd::String::hasTag(const char* str, const char* tag){
	if (!str) return 0;

	str = skipWhitespace(str);

	int len = strlen(tag);
	if (rd::String::equalsI(str, tag))
		return true;
	else {
		const char* nextComma = strchr(str, ',');
		if (!nextComma)//we already searched for a well formed tag
			return false;
		//const char* nextComma = strchr(str, ',');
		int tagLen = nextComma - str;
		if ( (tagLen==len) && 0==strnicmp(str, tag, len))
			return true;
		if (!nextComma) return false;//can't find comma
		if (!*nextComma) return false;//end of string
		if(!*(nextComma+1)) return false;//nothing understandable beyond comma
		return hasTag(nextComma + 1, tag);
	}
}

bool rd::String::hasTag(const std::vector<Str>& tags, const char* tag){
	StrRef refTag(tag);
	for (auto& t : tags) 
		if (rd::String::equalsI(t, refTag))
			return true;
	return false;
}

bool rd::String::hasTag(const eastl::vector<Str>& tags, const char* tag){
	StrRef refTag(tag);
	for (auto& t : tags) 
		if (rd::String::equalsI(t, refTag))
			return true;
	return false;
}

std::string std::to_string(std::any& val) {
	if (val.type() == typeid(int))
		return to_string(std::any_cast<int>(val));
	if (val.type() == typeid(float))
		return to_string(std::any_cast<float>(val));
	if (val.type() == typeid(double))
		return to_string(std::any_cast<double>(val));
	if (val.type() == typeid(bool))
		return to_string(std::any_cast<bool>(val));

	trace(val.type().name());

	return "unknown_any";
}
