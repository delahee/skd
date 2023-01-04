#include "stdafx.h"

#include "../LuaScriptHost.hpp"
#include "fmod.hpp"
#include "fmod_studio.hpp"

using namespace std;
using namespace r2;
using namespace rd;
using namespace Pasta; 

void LuaScriptHost::injectFmod(sol::state & luaSol) {
	luaSol.new_usertype<PointerWrapper<FMOD::Studio::EventDescription*>>("EventList",
		sol::constructors<PointerWrapper<FMOD::Studio::EventDescription*>(int)>(),
		//"GetPtr", &PointerWrapper<FMOD::Studio::EventDescription*>::GetPtr,			// DIDN'T WORK FOR NO REASON, use void* instead
		"GetVoidPtr", &PointerWrapper<FMOD::Studio::EventDescription*>::GetVoidPtr,
		"GetSize", &PointerWrapper<FMOD::Studio::EventDescription*>::GetSize,
		"GetAt", &PointerWrapper<FMOD::Studio::EventDescription*>::GetAt
	);

	auto fmodLua = luaSol["FMOD"].get_or_create<sol::table>();

	fmodLua.new_usertype<FMOD::Studio::EventDescription>("EventDescription",
		"getInstanceCount", [](FMOD::Studio::EventDescription* self) {
			int count = 0;
			self->getInstanceCount(&count);
			return count;
		},
		"releaseAllInstances", &FMOD::Studio::EventDescription::releaseAllInstances,
		"getPath", [](FMOD::Studio::EventDescription* self) {
			int size = 0;
			self->getPath(NULL, 0, &size);
			char* path = (char*)malloc(size);
			self->getPath(path, size, NULL);
			std::string res(path);
			free(path);
			return res;
		}
	);

	fmodLua.new_usertype<FMOD::Studio::Bank>("Bank",
		"getEventCount", [](FMOD::Studio::Bank* self) {
			int res = 0;
			self->getEventCount(&res);
			return res;
		},
		"getEventList", [](FMOD::Studio::Bank* self, void* eventList, int capacity) {
			int count = 0;
			self->getEventList((FMOD::Studio::EventDescription**)eventList, capacity, &count);
			return count;
		}
	);
}