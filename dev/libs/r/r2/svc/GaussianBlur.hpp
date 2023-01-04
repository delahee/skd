#pragma once

#include "r2/Tile.hpp"
#include "Capture.hpp"
#include "Kernel.hpp"

namespace r2 {
	namespace svc {

		struct GaussianBlurCtrl {
			r2::TexFilter filter	= r2::TexFilter::TF_NEAREST;
			bool		doPad		= false;
			r::Vector2	size		= r::Vector2(4, 4);
			r::Vector2	offsetScale = r::Vector2(1,1);

			void im();
		};

		class GaussianBlur {
		public:
			GaussianBlur();
			~GaussianBlur();
			static GaussianBlur&	get();

			r2::svc::Capture*		capture = nullptr;
			r2::svc::Capture*		paddedCapture = nullptr;
			r2::svc::Kernel*		kH = nullptr;
			r2::svc::Kernel*		kV = nullptr;
			
			r2::Tile*				make(r::Texture * n, const GaussianBlurCtrl & ctrl);
			r2::Tile*				make(r2::Tile * n, const GaussianBlurCtrl& ctrl);
			r2::Tile*				make(r2::Node * n, const GaussianBlurCtrl& ctrl);

			r2::Tile				dummy;
			r2::Tile				stub;
			r2::Tile				workingTile;

			int						forcedPad=-1;
			double					computedPad = 0.0;
			GaussianBlurCtrl		lastCtrl;
			void					im();

			static bool						recordTile;//not maintained
			static eastl::vector<r2::Tile>	debugTiles;//not maintained
		};
	}
}
