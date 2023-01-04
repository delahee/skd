#pragma once

#include "r/Types.hpp"

namespace rs {
	namespace Display {
		int width();
		int height();

		inline r::Vector2 getSize() {
			return r::Vector2(width(), height());
		};
		
		int initialWidth();
		int initialHeight();
	}
}