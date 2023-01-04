
#pragma once

#include"InputEvent.hpp"

namespace rs{
	class IEventListener {
		public:
			virtual void onEvent( InputEvent & ev) = 0;
	};
}