#pragma once

#include "1-graphics/FrameBuffer.h"
#include "r2/Scene.hpp"

namespace r2{
//optimization
// - ensure we call discard framebuffer_ext ?
	class OffscreenScene : public r2::Scene {
	public:
		RenderDoubleBuffer	rd;

		int					wantedWidth = 0;
		int					wantedHeight = 0;

		double				targetRatioH = 0.0;
		double				targetRatioW = 0.0;

		bool				targetFiltered = false;

							OffscreenScene(int wantedWidth, int wantedHeight, r2::TexFilter filter = r2::TexFilter::TF_NEAREST);
		virtual				~OffscreenScene();

		virtual void		render(Pasta::Graphic *g);

		Pasta::Texture*     getDepthTexture();
		r2::Tile*			getTargetTile();
		r2::Tile			dest;
	};
}