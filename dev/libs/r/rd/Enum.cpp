#include "stdafx.h"

#include "rd/Enum.hpp"

using namespace rd;

static rd::Enum * dummy = 0;

rd::EnumConstructor::EnumConstructor(const std::initializer_list<std::string>& l, int dfltIdx ){
	for(auto &id : l )
		list.push_back(Str(id));
	defaultIdx = dfltIdx;
}

void EnumList::im(){
	using namespace ImGui;
	int idx = 0;
	for(auto&e :content){
		ImGui::Text(std::to_string(idx)); SameLine();
		Value("id", e.id);
		if(e.vars.count()){
			Indent();
			e.vars.im(false,"params");
			Unindent();
		}
		idx++;
	}
}


EnumList& EnumList::operator+=(const char* name) {
	rd::Enum en;
	en.id = name;
	content.push_back(en);
	return *this;
}

static const char* parseEnumArg(const char* src, rd::Vars* into, int idx = 0) {
	int v = 0;
	float f = 0;
	src = rd::String::skipWhitespace(src);
	const char* rest = src;

	Str64 label;

	{//label scanning module ( var=val )
		const char* nextEqual = strstr(src, "=");
		const char* end = strchr(src, ')');
		const char* next = strchr(src, ',');
		const char* eq = strchr(src, '=');
		bool seekEqual = false;
		if ((next || end) && eq)
			seekEqual = true;
		if (seekEqual) {
			bool cont =
				((next && (eq < next)) //if there is a next it s after eq
					|| !next);//no next but an eq
			if (cont) {
				//if (sscanf_s(src, "%s=", label.c_str(), 255))
				int len = eq - src;
				strncpy(label.c_str(), src, len);
				label[len] = 0;

				rd::String::rtrimInPlace(label);

				src += len + 1;
				rest += len + 1;
			}
		}
	}

	if (!label.length())
		sprintf(label.c_str(), "%d", idx);

	if (rd::String::isInteger(src) && (sscanf(src, "%d", &v) != 0)) {
		into->set(label.c_str(), (int)v);
		return rd::String::skipToParenOrComma(rest);
	}
	else {
		f = 0.0f;
		if (sscanf(src, "%f", &f) != 0) {
			into->set(label.c_str(), (float)f);
			return rd::String::skipToParenOrComma(rest);
		}
		else {
			if (strstr(src, "\"")) {
				src = strstr(src, "\"");
				const char* end = strstr(src + 1, "\"");
				if (!end) return end;
				std::string strSub;
				strSub.assign(src + 1, end - src - 1);
				into->set(label.c_str(), strSub);
				return rd::String::skipWhitespace(end);
			}
			else if (strstr(src, "\'")) {
				src = strstr(src, "\'");
				src++;
				const char* end = strstr(src + 1, "\'");
				if (!end) return end;
				std::string strSub;
				strSub.assign(src, end - src);
				into->set(label.c_str(), strSub);
				return rd::String::skipWhitespace(end);
			}
			else if (strstr(src, "false")) {
				into->set(label.c_str(), false);
				return src + strlen("false");
			}
			else if (strstr(src, "true")) {
				into->set(label.c_str(), false);
				return src + strlen("true");
			}
			else {//it's an identifier
				src = rd::String::skipWhitespace(src);
				if ((!*src) || (*src == ')'))
					return src;
				const char* srcStart = src;
				while (*src && *src != ',' && *src != ')')
					src++;
				std::string strSub;
				strSub.assign(srcStart, src - srcStart);
				strSub = rd::String::trim(strSub);
				into->set(label.c_str(), strSub);
				return rd::String::skipWhitespace(src);
			}

		}
	}
}

rd::EnumList rd::Enum::parseEnumList(const std::string& src, r::c32 delim) {
	return parseEnumList(src.c_str(), delim);
}

rd::EnumList rd::Enum::parseEnumList(const char* src, c32 delim) {
	const char* start = rd::String::skipWhitespace(src);
	const char* cur = src;

	rd::EnumList res;
	if (!src) return res;
	if (!*src) return res;
	
	start = src;
	cur = start;
	do {
		auto p = rd::Enum::parseEnum(start, cur);
		if (p.id.empty())
			break;
		if (start == cur)
			break;
		start = rd::String::skipWhitespace(cur);
		res.content.push_back(p);
		start = rd::String::skipWhitespace(cur);
		if (!start || !*start)
			break;
		if (*start == delim)
			start++;
		start = rd::String::skipWhitespace(start);
	} while (*cur);
	return res;
}


void rd::Enum::im(){
	using namespace ImGui;
	ImGui::Text(id.c_str());
	vars.im(false);
}

rd::Enum rd::Enum::parseEnum(const char* src) {
	const char* nope = 0;
	return parseEnum(src, nope);
}

rd::Enum rd::Enum::parseEnum(const char* src, const char*& next) {
	rd::Enum p("");

	src = rd::String::skipWhitespace(src);
	if (!*src)
		return p;

	const char* start = src;
	const char* findFirstOpeningParen = strstr(src, "(");
	const char* findFirstSpace = strstr(src, " ");//rd::String::skipIdentifier
	bool useParenScan = false;

	if (!findFirstOpeningParen) {
		useParenScan = false;
	}
	else {
		if (findFirstSpace && findFirstOpeningParen) {
			if ( (findFirstOpeningParen-start) < (findFirstSpace-start))
				useParenScan = true;
		}
		if (!findFirstSpace && findFirstOpeningParen)
			useParenScan = true;
		
	}
	
	if (!useParenScan) {
		std::string l;

		const char* scan = start;
		while (*scan&&rd::String::isIdentifierCharacter(*scan))
			scan++;

		l.assign(start, scan - start);
		int len = strlen(src);
		if( len > 1)
		while(l.length() && (l[l.length() - 1] == ' '))
			l[l.length() - 1] = 0;
		l.resize(strlen(l.c_str()));

		p.id = l;
		if (next)
			next = src = rd::String::skipWhitespace(scan);

		return p;
	}
	else {
		if (findFirstOpeningParen == 0) {
			int here = 0;
		}
		const char* rest = findFirstOpeningParen + 1;
		std::string l = std::string().assign(start, rest - start - 1);
		l = rd::String::trim(l);
		p.id = Str(l);
		int argIdx = 0;
		do {
			rest = parseEnumArg(rest, &p.vars, argIdx);
			if (!rest)
				return p;
			rest = rd::String::skipWhitespace(rest);
			if (*rest == ')') {
				//finito
				next = rest + 1;
				return p;
			}
			rest = strstr(rest, ",");
			if (rest)
				rest++;
			argIdx++;
		} while (rest);

		if (next)
			next = rd::String::skipWhitespace(rest);
	}
	return p;
}

bool rd::EnumList::empty() const {
	return content.empty();
}

rd::Enum& rd::EnumList::get(const char* name) {
	for (auto& e : content)
		if ( rd::String::equalsI(e.id , name))
			return e;
	if (!dummy) {
		dummy = new rd::Enum();
		dummy->id = "dummy";
		dummy->vars.set("0", 0);
		dummy->vars.set("1", 0);
		dummy->vars.set("2", 0);
		dummy->vars.set("3", 0);
	}
	return *dummy;
}

bool rd::EnumList::has(const char* name) const{
	for (auto& e : content)
		if ( rd::String::equalsI(e.id , name))
			return true;
	return false;
}

bool rd::Enum::validate(EnumConstructor* _ctors){
	if (!_ctors) return false;

	ctors = _ctors;

	bool found = false;
	for (auto& e : _ctors->list)
		if (rd::String::equalsI(id, e.c_str())) {
			found = true;
			break;
		}
	if (!found) {
		if( !id.empty())
			trace(id.cpp_str() + " is not a member of constructors " + rd::String::join(_ctors->list,",").cpp_str());
		if(_ctors->defaultIdx>=0)
			id = _ctors->list[_ctors->defaultIdx];
	}
	return found;
}

int rd::Enum::i(int idx, int dfl) const {
	return vars.getInt(Str16f("%d", idx).c_str(), dfl);
}

float rd::Enum::f(int idx, float dfl) const {
	return vars.getInt(Str16f("%d", idx).c_str(), dfl);
}

const char* rd::Enum::s(const char* name, const char* dflt) {
	return vars.getString(name, dflt);
}

bool rd::Enum::operator==(const Enum& o) const {
	if (id != o.id)
		return false;
	if (!vars.isEqual(o.vars))
		return false;
	return true;
}

rd::Anon* rd::Enum::operator[](int idx){
	Str16f num("%d", idx);
	return vars.get(num.c_str());
}
