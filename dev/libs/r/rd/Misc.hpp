#pragma once

namespace rd {
	class LocalPrefs {
	public:
		static std::function<void(const char* propertyName, bool value)>	setPref;
		static std::function<bool(const char* propertyName, bool dflt)>		getPref;
	};
}