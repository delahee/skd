#pragma once

#include "r/Color.hpp"
#include "r2/Node.hpp"

namespace r2 {
	namespace ext {
		void texFilterNearest(r2::Node* spr);
		void texFilterLinear(r2::Node* spr);
		void blendAdd(r2::Node* spr);
	}
}