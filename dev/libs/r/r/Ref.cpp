#include "stdafx.h"
#include "r/Ref.hpp"

using namespace r;

bool r::Ref::DEBUG = false;

Ref::~Ref() { 
	PASTA_ASSERT_MSG(nbRef <= 0, "Unreleased object!");
}

int r::Ref::incrRef() { 
	return ++nbRef; 
}

void r::Ref::decRef() {
	PASTA_ASSERT(nbRef > 0); 
	if (nbRef > 0) { 
		nbRef--; 
		if (nbRef == 0) { 
			if(DEBUG|debug) traceObject("del ref", this);
			delete this; 
		} 
	} 
}

void r::Ref::im(const char * prefix){
	using namespace ImGui;
	Text(prefix);
	Indent();
	Text("Nb Ref %d", nbRef);
	Unindent();
}


bool r::SoftRef::DEBUG = false;

SoftRef::~SoftRef() {
	PASTA_ASSERT_MSG(nbRef <= 0, "Unreleased object!");
}

void r::SoftRef::im(const char* prefix){
	using namespace ImGui;
	Text(prefix);
	Indent();
	Text("Nb Ref %d", nbRef);
	Unindent();
}

void r::SoftRef::decRef() {
	PASTA_ASSERT(nbRef > 0);
	if (nbRef > 0) {
		nbRef--;
		if (nbRef == 0) {
			if (DEBUG) traceObject("del ref now", this);
		}
	}
}