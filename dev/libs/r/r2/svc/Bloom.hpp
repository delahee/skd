#pragma once

#include "Capture.hpp"
#include "r2/Tile.hpp"
#include "GaussianBlur.hpp"

namespace r2 {
	namespace svc {
		class Bloom {
		public:
			Bloom();

			//use the control for settings
			BloomCtrl					ctrl;

			//todo make a func that only return the blurred highpass result
			//give back a tile with color and bloom applied
			r2::Tile *					make(r2::Tile * tile);
			r2::Tile *					make(r::Texture * tex);

			void						bmpOpHighPass(r2::Bitmap & bmp);

			std::vector<BmpOp>			ops;
			r2::svc::Capture			highPass;
			r2::svc::GaussianBlur		highPassBlur;

			r2::svc::Capture			composite;

			r2::Tile					dummy;
			r2::Tile					stub;
			r2::Tile					workingTile;
			r2::Tile					drawingTile;
			void						im();
		};
	}
}