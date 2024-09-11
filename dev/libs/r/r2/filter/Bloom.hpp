#pragma once

#include "r/Types.hpp"
#include "r2/filter/Copy.hpp"
#include "r2/Tile.hpp"
#include "r2/svc/Bloom.hpp"

namespace r2 {
	namespace filter {
		class Bloom : public r2::filter::Copy {
			typedef r2::filter::Copy Super;
		public:
									Bloom();
			explicit				Bloom(const BloomCtrl& ctrl);
			virtual					~Bloom();

			BloomCtrl				ctrl;

			virtual r2::Tile*		filterTile(rs::GfxContext* g, r2::Tile* input);
			void					set(const BloomCtrl&ctrl);

			//can put false to use a doublebuffer optimisation
			//beware the delay may cause issues
			r2::Tile				workingTile;

			virtual void			im() override;

			virtual void			serialize(Pasta::JReflect& jr, const char* name) override;
			virtual r2::Filter*		clone(r2::Filter* obj = 0) override;

		protected:
			r2::svc::Bloom			bloom;
		};
	}
}