#include "stdafx.h"
#include "rd/Style.hpp"
#include "rui/xv/StyleSheet.hpp"

using namespace rui;
using namespace rui::xv;

StyleSheetEntry::StyleSheetEntry(const StyleSheetEntry& o) {
	sel = o.sel;
	st = new rd::Style(*o.st);
};

StyleSheetEntry& StyleSheetEntry::operator=(const StyleSheetEntry& o) {
	sel = o.sel;
	if (st) delete st;
	st = new rd::Style(*o.st);
	return *this;
};

