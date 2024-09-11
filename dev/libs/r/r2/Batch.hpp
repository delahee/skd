#pragma once

#include "Sprite.hpp"
#include "BatchElem.hpp"

namespace r2 {
	class BatchElem;

	struct BufferCache {
		BatchElem*					head = nullptr; // retrieve texture/blendmode info
		eastl::vector<float>		fbuf;
		eastl::vector<u32>			ibuf;
	};

	class Batch : public Sprite {
		typedef Sprite Super;
	protected:
		int							nbElems = 0;
	public:
		r2::BatchElem*				head = nullptr;
		bool						hasNormals = false;
	public:
									Batch(Node * parent = nullptr);
		virtual						~Batch();
		virtual void				dispose() override;

		virtual Tile *				getPrimaryTile();

		virtual void				draw(rs::GfxContext* g );

		virtual void				add(BatchElem* e);
		virtual BatchElem*			alloc(Tile* tile = nullptr, double _priority = 0.0);

		virtual void				remove(BatchElem* e);
		virtual void				release(BatchElem* e);

		bool						sanityCheck();

		virtual void				removeAllElements(); 
		virtual void				disposeAllElements();
		virtual void				deleteAllElements();

		//deprecated, use destroy that will "pick" the right disposal methode
		virtual void				poolBackAllElements();
		virtual void				destroyAllElements();

		void						traverseElements(std::function<void(BatchElem*)> visit);
		
		r2::BatchElem*				getFirstElement();
		r2::BatchElem*				getElement(int idx);
		r2::BatchElem*				getElementByName(const char* name);
		r2::BatchElem*				getElementByUID(r::uid _uid);
		r2::BatchElem*				getElementByTag(const char * tag);

		//fills a vector and splatter all elements in it
		std::vector<r2::BatchElem*> getAllElements();
		int							getNbElements();

		virtual Bounds				getMyLocalBounds() override;


		static void					unitTest();
		
		void						changePriority(r2::BatchElem* be, double nprio);

		virtual void				im() override;
		virtual Node*				clone(Node*nu=nullptr) override;

		virtual void				findByUID(r::u64 uid, r2::Node*& node, r2::BatchElem*& elem) override;

		virtual	NodeType			getType() const override { return NodeType::NT_BATCH; };

		static Batch*				fromPool(Node* parent = nullptr);
	protected:
		void						setTransparency(rs::GfxContext* g, BatchElem * elem);
		void						bindTile(rs::GfxContext* g, r2::Tile* t);
		void						setShader(rs::GfxContext* g);

		std::function<void(void)>	beforeDrawFlush;
		Tile*						currentRenderingTile = nullptr;
		int							nbSubmit = 0;

		BufferCache*				createBuffers(BatchElem* head);
		int							nbBuffers = 0;
		eastl::vector<BufferCache>	bufCache;
	};
}