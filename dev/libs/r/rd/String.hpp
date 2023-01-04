#pragma once

#include <vector>
#include <string>
#include <algorithm>
#include <unordered_map>
#include "r/Types.hpp"
#include "rd/Vars.hpp"
//#include "rd/Enum.hpp"
#include "EASTL/vector.h"
#include "r/Color.hpp"

namespace rd {
	class Vars;
	struct XmlData {
		std::string tag;
		std::unordered_map<std::string, std::string> attrs;
	};


	class String {
	public:
		static int						strlenUTF8(const char* input);
		static int						strlenUTF8(const std::string & input);

		static std::string				toString(const std::wstring& str);
		static std::wstring				toWString(const std::string& str);

		static const char*				skip(const char* forOpen, const char* toSkip);
		static const char*				skip(const std::string& forOpen, const char* toSkip);

		static std::vector<std::string>	split(const std::string & s, char delimiter);
		static std::vector<std::string>	split(const std::string & s, const std::string & delimiter);

		static std::string				join(const eastl::vector<std::string>& s, const std::string& delimiter);
		static Str						join(const eastl::vector<Str>& s, const Str& delimiter);

		static Str						join(const std::vector<Str>& s, const Str& delimiter);
		static std::string				join(const std::vector<std::string>& s, const std::string& delimiter);

		static std::string				replace(const std::string& original, const std::string& pattern, const std::string& newPattern);
		static Str						replace(const Str& original, const Str& pattern, const Str& newPattern);

		//replaces all occurences of tok by repl
		static void						replace(char *, char tok, char repl);

		static bool						getLastNumber(const char* str, int& _val);

		static char*					stristr(char* str, const char* Pattern);
		static const char*				stristr(const char* str, const char* Pattern);

		static bool						startsWithI(const char* str, const char* Pattern);
		static bool						startsWithI(const Str& str, const char* pattern);

		static bool						startsWith(const char* str, const char* Pattern);
		static bool						startsWith(const std::string& str, const char * pat);

		static inline
			bool						startsWith(const Str& str, const char* pattern) {
			return startsWith(str.c_str(), pattern);
		};

		static bool						contains(const std::string& string, const char* pattern);
		static bool						contains(const char* string, const char* pattern);
		static bool						containsI(const char* string, const char* pattern);
		static inline
			bool						containsI(const Str& string, const char* pattern){
			return containsI(string.c_str(), pattern);
		}

		static bool						hasTag(const char* string, const char* tag);
		static bool						hasTag(const Str& s, const char* tag) { return hasTag(s.c_str(), tag); };

		static bool						equals(const char* str0, const char* str1);
		static bool						equals(const std::string &str0, const char* str1);
		static bool						equals(const Str &str0, const char* str1);

		//as in case insensitive
		static bool						equalsI(const char* str0, const char* str1);
		static bool						equalsI(const std::string &str0, const char* str1);
		static bool						equalsI(const Str &str0, const char* str1);

		static inline bool endsWith(const std::string& str, const std::string& suffix) {
			return str.size() >= suffix.size() &&
				str.compare(str.size() - suffix.size(), suffix.size(), suffix) == 0;
		};

		static bool						endsWith(const char* str, const char* suffix);

		static void						parseXMLTags(const char* tag, XmlData & res);
		//todo migrate these to c32
		static std::string				getUtf8fromC32(int code);
		static std::string				c32toUtf8(int* codes, int forceLen = -1);

		//warning
		//returns the original string but modified
		//is not i18n correct see T::toLower
		static std::string&				toLower(std::string&);

		//warning
		//returns the original string but modified
		//is not i18n correct see T::toUpper
		static std::string&				toUpper(std::string&);

		static int 						c32Len(int * cpts);

		static int *					skipWhitespace(int * cpts);
		static const char*				skipWhitespace(const char* cpts);
		static void						toCodePoints(const char* utf8String, std::vector<int> & res);

		static Str						parseIdentifier(const char * str);
		static std::vector<Str>			parseIdentifierList(const char* str);
		static bool						parseBool(const char* str);

		static const char*				skipToParenOrComma(const char* src);
		static std::string				trim(const std::string& str);
		static void						rtrimInPlace(Str& s);

		static bool						isWhitespace(int cdpt);
		static bool						isInteger(const char* str);
		static bool						isFloat(const char* str);
		static bool						isBoolean(const char* str);
		static bool						isAlphaNum(int cdpt);
		static bool						isIdentifierCharacter(int cdpt);

		static r::Color					readColor(const char* lit);
	};
}

// cf 1-texts\StringUtils.cpp
#if defined(PASTA_WIN) || defined(PASTA_UWP) || defined(PASTA_XBOX) //seriously microsoft
#define strncasecmp _strnicmp
#define strcasecmp _stricmp
#endif