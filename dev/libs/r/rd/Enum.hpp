#pragma once

#include "rd/String.hpp"
#include "rd/Vars.hpp"

namespace rd {
	struct Enum;

	struct EnumList{
		eastl::vector<rd::Enum>	content;
		
		auto					begin() { return content.begin(); };
		const auto				cbegin() const { return content.cbegin(); };

		auto					end() { return content.end(); };
		const auto				cend() const { return content.cend(); };

		auto& operator[](int idx) {
			return content[idx];
		};
		const auto& operator[](int idx) const{
			return content[idx];
		};

		EnumList&				operator+=(const char* name);
		bool					has(const char* name) const;
		rd::Enum&				get(const char* name);
		bool					empty() const;
		void					im();
		size_t					size() { return content.size(); };

		void					debugPrint();
	};

	struct EnumConstructor{
		int					defaultIdx = 0;
		eastl::vector<Str>	list;

							EnumConstructor( const std::initializer_list<std::string> & l, int dltIdx = 0 );
	};

	struct Enum {
		Str					id;
		rd::Vars			vars;

		EnumConstructor*	ctors = 0;//used for validation

		Enum() { };
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

		int			i(int idx, int dfl = 0) const;
		float		f(int idx, float dfl = 0.0f) const;
		const char* s(const char* name = "0", const char* dflt = "");;

		inline bool is(const char * str) const {
			return rd::String::equalsI(id,str);
		};

		inline bool is(const std::string* str) const {
			return rd::String::equalsI(id, str->c_str());
		};

		inline bool is(const Str& str)  const {
			return rd::String::equalsI(id, str.c_str());
		};

		inline Enum& operator=(const Enum& o) {
			id = o.id;
			vars = o.vars;
			ctors = o.ctors;
			return *this;
		};

		bool operator==(const Enum& o) const;

		rd::Anon* operator[](int idx);

		void im();

		//parsed enum in the form of AAAAB or AA(x,y,z )
		static rd::Enum
			parseEnum(const char* src);
		//parsed enum in the form of AAAAB or AA(x,y,z )
		static rd::Enum
			parseEnum(const char* src, const char*& next);

		static rd::EnumList				parseEnumList(const char* src, r::c32 delim);
		static rd::EnumList				parseEnumList(const std::string& src, r::c32 delim);

		bool							validate(EnumConstructor * ctor);
	};
}

#define R_ENUM_DECL( v ) v,
#define R_ENUM_NAME( v ) #v,
#define R_LUA_DECL( v ) { #v, v },

#define R_ENUM_BITS_OP_OR( ty )\
static inline ty operator|(const ty& a, const ty& b) {\
	ty c = ty((int)a | (int)b);\
	return c;\
};

#define R_ENUM_BITS_OP_AND( ty )\
static inline ty operator&(const ty& a, const ty& b) {\
	ty c = ty((int)a & (int)b);\
	return c;\
};

#define R_ENUM_BITS_OP_EQOR( ty ) \
static inline ty& operator|=(ty& a, const ty& b) {\
	a = ty((int)a | (int)b);\
	return a;\
};

#define R_ENUM_BITS_OP_EQAND( ty ) \
static inline ty& operator&=(ty& a, const ty& b) {\
	a = ty((int)a & (int)b);\
	return a;\
};

#define R_ENUM_BITS_OPERATORS(ty) R_ENUM_BITS_OP_OR(ty) R_ENUM_BITS_OP_AND(ty) R_ENUM_BITS_OP_EQOR(ty) R_ENUM_BITS_OP_EQAND(ty)