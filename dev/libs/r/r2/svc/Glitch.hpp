#pragma once

#include "1-graphics/Texture.h"
#include "../RenderDoubleBuffer.hpp"
#include "Capture.hpp"

namespace r2 {
	namespace svc {
		class Glitch {
		public:
			//if you want to keep your own buffers
			Glitch();

			//allow to pool allocs
			Glitch & get();

			r2::Tile* make( r2::Node* src, const GlitchControl & ctrl);
			r2::Tile* make( r2::Tile* src, const GlitchControl & ctrl);
			r2::Tile* make( r::Texture* src, const GlitchControl & ctrl);

			r2::svc::Capture*		cCapture = nullptr;
			r2::svc::Capture*		cFilter = nullptr;
			std::vector<BmpOp>		ops;

			int						pad = 0;

			void					im();
		};

	}
}

