#pragma once

#include "Str.h"
#include "rd/String.hpp"
#include "rd/Vars.hpp"

namespace rd {
	struct Enum;

	struct EnumList{
		std::vector<rd::Enum> content;

		auto begin() {
			return content.begin();
		};

		auto end() {
			return content.end();
		};

		auto& operator[](int idx) {
			return content[idx];
		};

		bool has(const char* name);

		EnumList& operator+=(const char* name);
	};

	struct Enum {
		Str			id;
		rd::Vars	vars;

		Enum() {

		};

		Enum(const char* _id, const rd::Vars* v = nullptr) {
			id = _id;
			if (v)
				vars = *v;
		};

		Enum(const Enum& o) {
			id = o.id;
			vars = o.vars;
		};

		inline int i(const char * name, int dfl) {
			return vars.getInt(name, dfl);
		};

		inline bool is(const char * str) {
			return rd::String::equalsI(id,str);
		};

		inline bool is(const std::string* str) {
			return rd::String::equalsI(id, str->c_str());
		};

		inline bool is(const Str& str) {
			return rd::String::equalsI(id, str.c_str());
		};

		inline Enum& operator=(const Enum& o) {
			id = o.id;
			vars = o.vars;
			return *this;
		};

		void im();

		//parsed enum in the form of AAAAB or AA(x,y,z )
		static rd::Enum
			parseEnum(const char* src);
		//parsed enum in the form of AAAAB or AA(x,y,z )
		static rd::Enum
			parseEnum(const char* src, const char*& next);

		static rd::EnumList				parseEnumList(const char* src, r::c32 delim);
		static rd::EnumList				parseEnumList(const std::string& src, r::c32 delim);
	};
}

#define R_ENUM_DECL( v ) v,
#define R_ENUM_NAME( v ) #v,
