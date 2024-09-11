#pragma once
#include "Str.h"

namespace rplatform{
	class Device {
	public:
		static void			initPlatform();
		static Device*		me;
	};
}