#pragma once

#include "r2/svc/Capture.hpp"
#include "r2/Lib.hpp"

namespace r2 {
	namespace svc {
		class ColorMatrix {
		public:
			//if you want to keep your own buffers
			ColorMatrix();

			//allow to pool allocs
			ColorMatrix & get();

			r2::Tile* make( r2::Tile * src, const r2::ColorMatrixControl & ctrl);
			r2::Tile* make( r::Texture * src, const Pasta::Matrix44 & ctrl, r2::TexFilter filter);

			r2::svc::Capture		c;
			std::vector<BmpOp>		ops;
		};
	}
}

//cpp has a weir name because visual studio is stupid enough to not associate .obj with namespaces