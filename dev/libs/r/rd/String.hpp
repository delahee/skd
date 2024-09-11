#pragma once

#include <vector>
#include <string>
#include <any>
#include <unordered_map>
#include "r/Types.hpp"
#include "EASTL/vector.h"
#include "r/Color.hpp"

namespace rd {
	class Vars;
	struct XmlData {
		Str	tag;
		std::unordered_map<Str, Str>	
			attrs;
	};

	class String {
	public:
		static int						strlenUTF8(const char* input);
		static int						strlenUTF8(const std::string & input);

		static std::string				toString(const std::wstring& str);
		static std::wstring				toWString(const std::string& str);

		static const char*				skipTo(const char* forOpen, int code);
		static const char*				skipTo(const char* forOpen, const char* toSkip);
		static const char*				skip(const char* forOpen, const char* toSkip);
		static const char*				skip(const std::string& forOpen, const char* toSkip);

		static std::vector<std::string>	split(const std::string & s, char delimiter);
		static std::vector<std::string>	split(const std::string & s, const std::string & delimiter);
		static eastl::vector<Str>		splitStr(const char *s, const char * delimiter);

		static std::string				join(const eastl::vector<std::string>& s, const std::string& delimiter);
		static Str						join(const eastl::vector<Str>& s, const Str& delimiter);

		static Str						join(const std::vector<Str>& s, const Str& delimiter);
		static std::string				join(const std::vector<std::string>& s, const std::string& delimiter);

		static std::string				join(const char** s, int nb, const std::string& delimiter);

		static std::string				replace(const std::string& original, const std::string& pattern, const std::string& newPattern);
		//static std::string				replace(const std::string& original, const char* pattern, const char* newPattern);

		//static Str						replace(const Str& original, const char* pattern, const char* newPattern);
		static Str						replace(const Str& original, const Str& pattern, const Str& newPattern);

		static int						find(const Str&s, const char * pattern);
		static int						rfind(const Str&s, const char * pattern);

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

		static bool						contains(const char* string, char pattern);
		static bool						contains(const std::string& string, char pattern) { return contains(string.data(), pattern); };
		static bool						contains(const Str& string, char pattern);

		static bool						contains(const std::string& string, const char* pattern);
		static bool						contains(const char* string, const char* pattern);
		static bool						contains(const Str& string, const char* pattern);
		static bool						containsI(const char* string, const char* pattern);
		static inline
			bool						containsI(const Str& string, const char* pattern) {
			return containsI(string.c_str(), pattern);
		};
		
		static inline
			bool						containsI(const std::string& string, const std::string& pattern) {
			return containsI(string.c_str(), pattern.c_str());
		};

		static bool						addTag(Str& string, const char* tag);
		static bool						hasTag(const char* string, const char* tag);
		static bool						hasTag(const Str& s, const char* tag) { return hasTag(s.c_str(), tag); };
		static bool						hasTag(const std::vector<Str>& tags, const char* tag);
		static bool						hasTag(const eastl::vector<Str>& tags, const char* tag);
		
		static bool						equals(const char* str0, const char* str1);
		static bool						equals(const std::string &str0, const char* str1);
		static bool						equals(const Str &str0, const char* str1);

		//as in case insensitive
		static bool						equalsI(const char* str0, const char* str1);
		static bool						equalsI(const std::string &str0, const char* str1);
		static bool						equalsI(const Str &str0, const char* str1);
		static bool						equalsI(const Str& str0, const Str& str1);


		static inline bool				endsWith(const std::string& str, const char* suffix) { return endsWith(str.c_str(), suffix); };
		static bool						endsWith(const Str& str, const char* suffix) { return endsWith(str.c_str(), suffix); };
		static bool						endsWith(const char* str, const char* suffix);
		static bool						endsWithI(const char* str, const char* suffix);

		static void						parseXMLTags(const char* tag, XmlData & res);
		//todo migrate these to c32
		static std::string				getUtf8fromC32(int code);


		static int						utf8code(const char* lit);
		static eastl::vector<int>		utf8ToC32(const std::string & str);
		static std::string				c32toUtf8(int* codes, int forceLen = -1);

		/**warning
		returns the original string but modified
		is not i18n correct and not utf8 correct see T::toLower
		**/
		static bool						isLower(std::string& str );
		static std::string&				toLower(std::string& str);

		/**warning
		returns the original string but modified
		is not i18n correct see T::toLower
		**/
		static void						lower(char* str, int count);

		/**warning
		returns the original string but modified
		is not i18n correct see T::toLower
		**/
		static void						upper(char* str, int count);

		//warning
		//returns the original string but modified
		//is not i18n correct see T::toUpper
		static std::string&				toUpper(std::string&);

		//is not i18n correct see T::capitalize/toCapitalized if they exist
		static std::string&				capitalize(std::string&);
		static std::string				toCapitalized(const std::string&);
		static std::string&				capitalizeFirstChar(std::string& str);


		static std::string&				camelCase(std::string&);
		static std::string				toCamelCase(const std::string&);

		static int 						c32Len(int * cpts);

		static int *					skipWhitespace(int * cpts);
		static const char*				skipWhitespace(const char* cpts);
		static void						toCodePoints(const char* utf8String, std::vector<int> & res);
		static void						toCodePoints(const char* utf8String, eastl::vector<int> & res);

		static Str						parseIdentifier(const char * str);
		static eastl::vector<Str>		parseIdentifierList(const char* str);
		static eastl::vector<int>		parseIntList(const char* str);
		static eastl::vector<int>		parseIntList(const char* str,const char * sep);
		static bool						parseBool(const char* str);

		static double					parseFloat(const char* str, double dflt=0);
		static double					parseFloat(const char* str, double dflt, const char* &end);

		static int						parseInt(const char* str, int dflt=0);
		static int						parseInt(const char* str, int val, const char*& end);


		static const char*				skipToParenOrComma(const char* src);


		static const char*				ltrim(const char*s);
		static std::string				trim(const std::string& str);
		static void						rtrimInPlace(Str& s);

		static bool						isWhitespace(int cdpt);
		static bool						isInteger(const char* str);
		static bool						isFloat(const char* str);
		static bool						isBoolean(const char* str);
		static bool						isAlphaNum(int cdpt);
		static bool						isNum(int cdpt);
		static bool						hasFloatSymbols(char* str);
		static bool						isNumFloat(int cdpt);
		static bool						isIdentifierCharacterEmojiCompat(int cdpt);
		static bool						isIdentifierCharacter(int cdpt);
		static bool						isEmpty(const char * str);
		static bool						isColon(const char * str);
		static bool						isNewline(int code);

		static r::Color					readColor(const char* lit);
		static Str						sub(const char* s, const char* end);
		static std::string				empty;
		static Str						emptyStr;
	};
}

// cf 1-texts\StringUtils.cpp
#if defined(PASTA_WIN) || defined(PASTA_UWP) || defined(PASTA_XBOX) //seriously microsoft
#define strncasecmp _strnicmp
#define strcasecmp _stricmp
#endif

const char* strrstr(const char* haystack, const char* needle);

namespace std {
	inline std::string		replace(const std::string& src, const std::string& nu, const std::string& ptrn) { return rd::String::replace(src,nu,ptrn); };
	std::string					to_string(std::any& val);
}
	