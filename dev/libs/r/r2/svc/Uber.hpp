#pragma once

#include <vector> 
#include "r/Types.hpp"
#include "r2/Filter.hpp"
#include "r2/svc/Capture.hpp"

namespace r2 {
	namespace svc {
		class Uber : public r2::svc::Capture {
			std::vector<Pasta::ShaderProgram*> pyramidShader;
			std::vector<r2::SingleFbPage*> pyramidBuffer;
			Scene* tmpSc = nullptr;
		public:
			Uber();

			r2::SingleFbPage* computeBloomPyramid(r::Texture* tex);

			r2::Tile* make(r::Texture* tex, r2::TexFilter filter = r2::TexFilter::TF_INHERIT);
			void blit(rs::GfxContext* gfx, const Pasta::Matrix44& trs, r2::Tile* tile);

			bool distortionEnabled = false;
			Pasta::Vector4 distortionAmount = Pasta::Vector4(0, 0, 1, 0);
			float downloadTransitionAmount = 1.0f;

			bool glitchEnabled = false;
			Pasta::Vector4 glitchAmount = Pasta::Vector4(0, 0, 0, 4);

			bool chromaticEnabled = false;
			Pasta::Vector2 chromaticAberrationSettings = Pasta::Vector2(0.05f, 5);

			bool bloomEnabled = false;
			float bloomThreshold = 0.5;
			float bloomKnee = 0.5;
			float bloomIntensity = 1;
			int pyramidSize = 5;
			float bloomScale = 1;
			r::Color bloomColor = r::Color(1,1,1,1);

			bool vignetteEnabled = false;
			Pasta::Vector4 vignetteAmount = Pasta::Vector4(1, 1, 1, 0);
			r::Color vignetteColor = r::Color(0, 0, 0, 1);

			bool colorMatrixEnabled = false;
			ColorMatrixControl colorMatrixCtrl;

			bool dissolveEnabled = false;
			float dissolveAmount = 0.0f;
			float dissolveZoom = 2.0f;

			std::vector<BmpOp>		ops;

			void					bmpOp(r2::Bitmap& bmp, r::Texture* pyramidResult);
			void					im();
			void					upload();
		};
	}
}
