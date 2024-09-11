#pragma once

namespace rd {
	class StaticBatch : public r2::Batch {
	public:
		StaticBatch(r2::Node * parent = nullptr);

		virtual					~StaticBatch() {};
		virtual void			draw(rs::GfxContext * g) override;

		virtual void			add(r2::BatchElem * e) override;
		virtual r2::BatchElem*	alloc(r2::Tile * tile, double _priority = 0.0) override;

		void					remove(r2::BatchElem * e) override;
		void					release(r2::BatchElem * e) override;

		virtual void			removeAllElements() override;
		virtual void			disposeAllElements() override;
		virtual void			deleteAllElements() override;
		virtual void			poolBackAllElements() override;
		virtual void			destroyAllElements() override;

		void					invalidate();

	protected:
		bool						computed = false;
		int							triangleCount = 0;
		eastl::vector<float>		fbuf;
		eastl::vector<u32>			ibuf;
		Pasta::Texture *			texture = nullptr;
		Pasta::TransparencyType		blendMode = Pasta::TT_ALPHA;
	};
}