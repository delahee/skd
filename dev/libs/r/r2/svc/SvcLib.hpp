#pragma once
#include "1-graphics/Texture.h"
#include "EASTL/vector.h"
#include "r/Types.hpp"

namespace r2 {
	namespace svc {
		class SvcLib {
		public:
			static void copyTexSubImage2D(
				eastl::vector<r::u32>&	pixels,
				Pasta::TextureData*		data,
				int						x,
				int						y,
				int						w,
				int						h
			);
		};
	}
}