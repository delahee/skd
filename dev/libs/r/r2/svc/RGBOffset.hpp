#pragma once

#include <vector> 
#include "r/Types.hpp"
#include "r2/Filter.hpp"
#include "r2/svc/Capture.hpp"

namespace r2 {
	namespace svc {
		class RGBOffset : public r2::svc::Capture {
		public:
			RGBOffset();

			//offset must be in uv space
			//todo take a tile as well
			r2::Tile * make(r::Texture * tex, const float * rgbOfs, r2::TexFilter filter = r2::TexFilter::TF_INHERIT);

			std::vector<BmpOp>		ops;
			std::vector<float>		rgbOfs;

			void					bmpOp(r2::Bitmap&bmp);
		};
	}
}
