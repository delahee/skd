#pragma once
#include "Sprite.hpp"
#include "1-graphics/geo_vectors.h"
#include "EASTL/vector.h"

namespace rs { class GfxContext; }
namespace Pasta { class Texture; }

namespace r2 {
	struct PatchElem {
		r2::Tile			t;
		Pasta::Vector3		pos;
	};

	//todo add static version
	class Patch : public r2::Sprite {

	public:
		Patch(r2::Node* parent = nullptr) : r2::Sprite(parent) {  };
		virtual						~Patch() {};
		virtual void				process(rs::GfxContext* g);
		void						setTransparency(rs::GfxContext* _g);
		virtual void				draw(rs::GfxContext* g);
		virtual void				add(const PatchElem& e);
		virtual r2::Bounds			getMyLocalBounds() override;
		virtual void				im() override;
		virtual	NodeType			getType() const override { return NodeType::NT_PATCH; };
		virtual Texture*			getTexture(rs::GfxContext* _g, int slot) override;
	protected:
		bool						shouldRender(rs::GfxContext* _g);
		eastl::vector<PatchElem>	elems;
		eastl::vector<float>		vbuf;
		eastl::vector<r::u32>		ibuf;
	};
}