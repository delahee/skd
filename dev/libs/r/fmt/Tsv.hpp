#pragma once

#include <string>
#include <vector>
#include <functional>
#include "rd/Enum.hpp"

namespace fmt{
class Tsv {
public:
	static std::vector<std::string>	parse(std::string content, char sep);
	static std::vector<std::string>	extractLines(const char * file);
	static std::vector<std::string>	extractColumns(const char * line);
	static void						parseAndAssign(const std::string& path, std::function<void(std::vector<std::string>& _prm)> assign);
	static void 					parseAndAssign(const char* path, std::function<void(std::vector<std::string>& _prm)> assign);
	static void						watchParseAndAssign(const char* path, std::function<void(std::vector<std::string>& _prm)> assign);

	template<typename T>
	static inline T parse(std::vector<std::string>& line, int idx, T dflt) {
		return dflt;
	}

	static Vector2i parseSplitInt2(std::vector<std::string>& line, int idx) {
		if (idx < 0) return Vector2i();
		Vector2i ret;
		int nb = sscanf(line[idx].data(), "%d/%d", &ret.x, &ret.y);

		return ret;
	}

	static std::vector<Str> parseIdentifierList(std::vector<Str>& line, int idx);
	static std::vector<Str> parseIdentifierList(std::vector<std::string>& line, int idx);
	static rd::EnumList		parseEnums(std::vector<std::string>& line, int idx);
	static rd::Enum			parseEnum(std::vector<std::string>& line, int idx);

	static rd::Vars*		parseVars(const Str& src);

	static rd::Vars*		parseVars(std::vector<std::string>& line, int idx){
		if (idx < 0) return nullptr;
		auto& p = line[idx];
		if (p.empty()) return nullptr;
		return parseVars(StrRef(p));
	} 
};

template<>
inline std::string	Tsv::parse(std::vector<std::string>& line, int idx, std::string dflt) {
	if (idx < 0)return dflt;
	return line[idx];
}

template<>
inline const char*	Tsv::parse(std::vector<std::string>& line, int idx, const char* dflt) {
	if (idx < 0) return dflt;
	return line[idx].c_str();
}

template<>
inline int			Tsv::parse(std::vector<std::string>& line, int idx, int dflt) {
	if (idx < 0)return dflt;
	int ret = dflt;
	sscanf(line[idx].data(), "%d", &ret);
	return ret;
}

template<>
inline float		Tsv::parse(std::vector<std::string>& line, int idx, float dflt) {
	if (idx < 0) return dflt;
	float ret = dflt;
	char* lit = line[idx].data();
	if (rd::String::contains(lit, ",") && !rd::String::contains(lit, ".")) {
		//assume it'sd a goddamn french number
		rd::String::replace(lit, ',', '.');
	}
	int nb = sscanf(lit, "%f", &ret);
	if (nb == 0) return dflt;
	return ret;
}

template<>
inline double		Tsv::parse(std::vector<std::string>& line, int idx, double dflt) {
	if (idx < 0) return dflt;
	double ret = dflt;
	char* lit = line[idx].data();
	if (rd::String::contains(lit, ",") && !rd::String::contains(lit, ".")) {
		//assume it'sd a goddamn french number
		rd::String::replace(lit, ',', '.');
	}
	int nb = sscanf(lit, "%lf", &ret);
	if (nb == 0) return dflt;
	return ret;
}

template<>
inline bool		Tsv::parse(std::vector<std::string>& line, int idx, bool dflt) {
	if (idx < 0) return dflt;
	auto& p = line[idx];
	if (0 == strcmp(p.data(), "1"))
		return true;
	if (0 == strcasecmp(p.data(), "true"))
		return true;
	if (0 == strcasecmp(p.data(), "vrai"))//thank you google...
		return true;
	if (0 == strcmp(p.data(), "0"))
		return false;
	if (0 == strcasecmp(p.data(), "false"))
		return false;
	if (0 == strcasecmp(p.data(), "faux"))//thank you google...
		return false;
	return dflt;
}

template<>
inline r::Vector2i	Tsv::parse(std::vector<std::string>& line, int idx, r::Vector2i dflt) {
	if (idx < 0)return dflt;
	auto& p = line[idx];
	Vector2i res(0, 0);
	int nbScanned = sscanf(p.c_str(), "%i,%i", &res.x, &res.y);
	if (!nbScanned)
		nbScanned = sscanf(p.c_str(), "%i/%i", &res.x, &res.y);
	if (!nbScanned)
		res = dflt;
	return res;
}

template<>
inline r::Vector2	Tsv::parse(std::vector<std::string>& line, int idx, r::Vector2 dflt) {
	if (idx < 0)return dflt;
	auto& p = line[idx];
	Vector2 res(0, 0);
	int nbScanned = sscanf(p.c_str(), "%f,%f", &res.x, &res.y);
	if (!nbScanned)
		nbScanned = sscanf(p.c_str(), "%f/%f", &res.x, &res.y);
	if (!nbScanned)
		res = dflt;
	return res;
}

}//end namespace fmt