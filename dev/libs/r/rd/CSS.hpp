#pragma once

#include "r/Color.hpp"
#include "Vars.hpp"

namespace rd {

#define IMPL_I( varname ) \
	int varname() {\
		if (vals.has( (#varname) )) \
			return vals.getInt( (#varname) ); \
		if (parent) return parent->varname(); \
		return {};\
	};\
\
	void set_##varname( int val ) {\
		vals.set( (#varname), val);\
	};\
 
/*
#define IMPL_COLOR( varname ) \
	r::Color varname() {\
		if (vals.has( (#varname) )) \
			return vals.getColor( (#varname) ); \
		if (parent) return parent->varname(); \
		return {};\
	};\
\
	void set_##varname( const r::Color & val ) {\
		vals.set( (#varname), val);\
	};\

*/

#define IMPL_PTR( tp, varname ) \
	tp* varname() {\
		if (vals.has( (#varname) )) \
			return (tp*) vals.getPtr( (#varname) ); \
		if (parent) return (tp*) parent->varname(); \
		return {};\
	};\
\
	void set_##varname( tp * val ) {\
		vals.set( (#varname), (void*)val);\
	}; \
 
	/**untested... not complete, do NOT NUSE... lamish why are you making me do this to myself...
	*/
	struct CSS {
		rd::Vars					vals;

		std::vector<std::string>	sel;
		CSS*						parent = nullptr;

		~CSS() {
			if (parent)
				delete parent;
			parent = nullptr;
		};

		IMPL_I(fontSize)
		//IMPL_COL(fontColor)
		IMPL_PTR(rd::Font, fontFamily)

		
	};
}