#pragma once

#include "r/Types.hpp"
#include "r2/Filter.hpp"
#include "r2/Tile.hpp"
#include "r2/filter/Layer.hpp"
#include "r2/filter/Copy.hpp"
#include "r2/svc/Kernel.hpp"

namespace r2 {
	namespace filter {
		class Blur : public r2::filter::Copy {
			typedef r2::filter::Copy  Super;
		public:
									Blur();
			explicit				Blur(const Vector2 & size, float scale = 1.0, r2::TexFilter filter = r2::TexFilter::TF_NEAREST);
			virtual					~Blur();

			float					offsetScale = 1.0f;
			float					resolutionDivider = 1.0f;
			Pasta::Vector2			size = Pasta::Vector2(4, 4);
			r2::Tile				workingTile;

			void					updateSize(); 
			virtual r2::Tile *		filterTile(rs::GfxContext * g, r2::Tile * input);
			void					set(const Vector2 & size, float scale = 1.0, r2::TexFilter  filter = r2::TexFilter::TF_NEAREST);
			virtual void			im() override;

			virtual void			invalidate() override;

			virtual void			serialize(Pasta::JReflect& jr, const char* name) override;
			virtual r2::Filter*		clone(r2::Filter* obj = 0) override;
			
		protected:
			r2::svc::Kernel* kH = nullptr;
			r2::svc::Kernel* kV = nullptr;
			
		};
	}
}