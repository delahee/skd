#pragma once

#include "r2/Types.hpp"
#include "r2/filter/Blur.hpp"

namespace r2{
	namespace filter {
		class Glow : public r2::filter::Blur {
		public:
									Glow();
									Glow(const Vector2& size,
										const Color& color,
										float scale,
										r2::TexFilter filter,
										bool knockout = false);

			virtual					~Glow();

			void					set(
				const Vector2& size,
				const Color& color,
				float scale, 
				r2::TexFilter filter, 
				bool knockout = false);

			virtual r2::Tile *		filterTile(rs::GfxContext * g, r2::Tile * input);
			virtual void			im() override;
			virtual void			serialize(Pasta::JReflect&jr,const char * name) override;
			virtual r2::Filter*		clone(r2::Filter* obj = 0) override;


			bool					knockout = false;
			bool					matrixMode = false;

			Bitmap *				bClear	= nullptr;
			Bitmap *				bGlow	= nullptr;

			r::Color				glowColor = r::Color(1, 0, 0, 1);
			r::Color				glowColorBlit = r::Color(1, 1, 1, 1);
			float					glowIntensity = 8;

			TexFilter				compositingFilter = TexFilter::TF_NEAREST;

			//beware transposed by code before sending to shader
			r::Matrix44				colorMatrix;
		};
	}
}