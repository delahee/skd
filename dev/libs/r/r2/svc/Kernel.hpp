#pragma once

#include "1-graphics/Texture.h"
#include "r2/RenderDoubleBuffer.hpp"
#include "r2/Tile.hpp"
#include "Capture.hpp"
#include "EASTL/vector.h"

namespace r2 {
	namespace svc {
		class Kernel {
		public:	
								Kernel();
								~Kernel();
			Kernel &			get();

			r2::Tile *			makeBlur1D(Pasta::Texture * src, double size, double scale, double resolutionDivider, r2::TexFilter filter, bool horizontalOrVertical);
			r2::Tile *			makeBlur1D(r2::Node * src, double size, double scale, double resolutionDivider, r2::TexFilter filter, bool horizontalOrVertical);

			//reset the texture to 0 0 
			r2::Tile *			makeBlur1D(r2::Tile * src, double size, double scale, double resolutionDivider, r2::TexFilter filter, bool horizontalOrVertical);

			//keeps dx and dy which saves a bit of space
			r2::Tile *			makeBlur1D_KeepOffset(r2::Tile* src, double size, double scale, r2::TexFilter filter, bool horizontalOrVertical);


			RenderDoubleBuffer	rd;
			r2::svc::Capture*	nodeCapture = nullptr;
			r2::Tile			dummy;
			r2::Tile			stub;

			eastl::vector<float> kernel;
			eastl::vector<float> offsets;//layout {x y} {x y}

			static int			getKernelWidth(double size) { return (int)(size / 0.65f + 0.5f) * 2 + 1; };

			void				im();
			r2::Tile*			getWorkingTile() { return rd.getWorkingTile(); };
		};
	}
}
