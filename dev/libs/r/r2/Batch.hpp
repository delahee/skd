#pragma once

#include <vector>
#include "EASTL/vector.h"
#include "Sprite.hpp"
#include "BatchElem.hpp"
#include "rs/GfxContext.hpp"

namespace r2 {
	class BatchElem;
	class Batch : public Sprite {

	public:
		r2::BatchElem*	head = nullptr;
		int				nbElems = 0;
		int				nbSubmit = 0;
		bool			hasNormals = false;
	public:
						Batch(Node * parent = nullptr);
		virtual			~Batch();
		virtual void	dispose() override;

		virtual Tile *	getPrimaryTile();

		virtual void	draw( rs::GfxContext * g );

		virtual void			add(BatchElem * e);
		virtual BatchElem*		alloc(Tile * tile = nullptr, double _priority = 0.0);

		virtual void	remove(BatchElem * e);
		virtual void	free(BatchElem * e);

		bool			sanityCheck();

		virtual void	removeAllElements(); 
		virtual void	disposeAllElements();
		virtual void	deleteAllElements();

		//deprecated, use destroy that will "pick" the right disposal methode
		virtual void	poolBackAllElements();
		virtual void	destroyAllElements();

		void			traverseElements(std::function<void(BatchElem*)> visit);

		r2::BatchElem*	getElement(int idx);
		r2::BatchElem*	getElementByName(const char * name);

		//fills a vector and splatter all elements in it
		std::vector<r2::BatchElem*> getAllElements();
		int				getNbElements();

		virtual Bounds	getMyLocalBounds() override;

		r2::BatchElem*	firstElem();
		r2::BatchElem*	getByUID(r::uid _uid);

		static void		unitTest();
		
		void			changePriority(r2::BatchElem * be, double nprio);

		virtual void	im() override;
		virtual Node*	clone(Node*nu=nullptr) override;

		virtual void	findByUID(r::u64 uid, r2::Node * & node, r2::BatchElem * & elem) override;

		virtual	NodeType getType() const override { return NodeType::NT_BATCH; };

		static Batch*	fromPool(Node* parent = nullptr);
	protected:
		void			setTransparency(rs::GfxContext* g, BatchElem * elem);
		void			bindTile(rs::GfxContext* g, r2::Tile* t);
		void			setShader(rs::GfxContext* g);

		std::function<void(void)>	beforeDrawFlush;
		eastl::vector<float>		fbuf;
		eastl::vector<u32>			ibuf;
		Tile*						currentRenderingTile = nullptr;
	};
}