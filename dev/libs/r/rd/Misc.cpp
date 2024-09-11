#include "stdafx.h"
#include "Misc.hpp"

std::function<void(const char* propertyName, bool value)>	
	rd::LocalPrefs::setPref;

std::function<bool(const char* propertyName, bool dflt)>				
	rd::LocalPrefs::getPref;

