#include "stdafx.h"

#include "rui/xv/StyleSheet.hpp"

void rui::xv::Selector::setClasses(const char* cls) {
	classes = rd::String::splitStr(cls, " ");
	eastl::sort(classes.begin(), classes.end(), [](const Str& a, const Str& b) {
		return a < b;
	});
}

//on the left is the style sheet rule
bool rui::xv::Selector::matchesClasses(const Selector& sel) const {
	for (auto& c : classes) 
		if( rs::Std::exists(sel.classes, c.c_str()))
			return true;
	return false;
}

bool rui::xv::Selector::matches(const Selector& sel) const {
	if (id == sel.id && sel.classes.empty())
		return true;
	return matchesClasses(sel);
};