#include "stdafx.h"

#include "Pools.hpp"

using namespace rd;
using namespace r2;

template class rs::Pool<TileAnim>;
template class rs::Pool<r2::Tile>;
template class rs::Pool<rd::ABatchElem>;
template class rs::Pool<rd::ABitmap>;
template class rs::Pool<rd::SubBatchElem>;
template class rs::Pool<rd::SubABatchElem>;
template class rs::Pool<r2::Bitmap>;
template class rs::Pool<r2::Node>;
template class rs::Pool<r2::Text>;
template class rs::Pool<r2::Graphics>;
template class rs::Pool<r2::Batch>;
template class rs::Pool<rd::StaticBatch>;
template class rs::Pool<Pasta::ShadedTexture>;
template class rs::Pool<r2::BatchElem>;
template class rs::Pool<rd::AnonAgent>;
template class rs::Pool<r2::Interact>;
template class rs::Pool<rd::Anon>;

rs::Pool<TileAnim>						Pools::anims;
rs::Pool<r2::Tile>						Pools::tiles;
rs::Pool<rd::ABatchElem>				Pools::aelems;
rs::Pool<rd::SubBatchElem>				Pools::subelems;
rs::Pool<rd::SubABatchElem>				Pools::subaelems;
rs::Pool<r2::Bitmap>					Pools::bitmaps;
rs::Pool<rd::ABitmap>					Pools::abitmaps;
rs::Pool<r2::Node>						Pools::nodes;
rs::Pool<r2::Text>						Pools::texts;
rs::Pool<r2::Graphics>					Pools::graphics;
rs::Pool<r2::Batch>						Pools::batches;
rs::Pool<rd::StaticBatch>				Pools::sbatches;
rs::Pool<Pasta::ShadedTexture>			Pools::imTexHolders;

rs::Pool<r2::BatchElem>					Pools::elems;
rs::Pool<r2::Interact>					Pools::interacts;
rs::Pool<rd::AnonAgent>					Pools::aa;
rs::Pool<rd::Anon>						Pools::anons;

eastl::vector<Pasta::ShadedTexture*>	Pools::deleteList;
eastl::vector<void*>					Pools::rawFreeList;

static constexpr bool TRACK_POOLING = false;

static void commonAlloc(r2::Node* b) {
	//if (TRACK_POOLING) trace(std::string("allocing ") + b->name + "-" + std::to_string(b->uid));
	b->nodeFlags &= ~NF_IN_POOL;
	b->reset();
	b->nodeFlags |= NF_ORIGINATES_FROM_POOL;
}

static void commonAlloc(r2::BatchElem* b) {
	//if (TRACK_POOLING) trace(std::string("allocing ") + b->name + "-" + std::to_string(b->uid));
	b->blendmode = Pasta::TT_INHERIT;
	b->beFlags &= ~NF_IN_POOL;
	b->reset();
	b->beFlags |= NF_ORIGINATES_FROM_POOL;
	b->setTile( r2::Tile::fromPool(nullptr), true );
}

static void commonFree(r2::Node* b) {
	//if (TRACK_POOLING) trace(std::string("freeing ") + b->name + "-" + std::to_string(b->uid));
	b->nodeFlags |= NF_IN_POOL;
	b->dispose();
	b->invalidateTRS();
}

static void commonFree(r2::BatchElem* b) {
	//if (TRACK_POOLING) trace(std::string("freeing ") + b->name + "-" + std::to_string(b->uid));
	b->clear();	
	b->dispose();
	b->beFlags |= NF_IN_POOL;
	b->z = b->y = b->x = -66 * 1024 * 1024;
}

static void commonFree(r2::Sprite* b) {
	//if (TRACK_POOLING) trace(std::string("freeing ") + b->name + "-" + std::to_string(b->uid));
	b->blendmode = TransparencyType::TT_ALPHA;
	b->nodeFlags |= NF_IN_POOL;
	b->dispose();
	b->invalidateTRS();
}

static void commonFree(r2::Graphics* b) {
	//if (TRACK_POOLING) trace(std::string("freeing ") + b->name + "-" + std::to_string(b->uid));
	b->clear();
	b->blendmode = TransparencyType::TT_ALPHA;
	b->nodeFlags |= NF_IN_POOL;
	b->dispose();
	b->invalidateTRS();
}


struct _Init {
	_Init() {
		Pools::anims.onFree = [](TileAnim * t) {
			t->clear();
		};

		static int break_Tile = 3656;

		Pools::anons.reserve(32);
		Pools::anons.onAlloc = [](rd::Anon* t) {
			t->flags |= AFL_POOLED;
		};
		Pools::anons.onFree = [](rd::Anon* t) {
			t->dispose();
			t->flags = AFL_POOLED;
		};

		Pools::tiles.reserve(32);
		Pools::tiles.onAlloc = [](r2::Tile *t) {
			//if( TRACK_POOLING ) trace(std::string("allocing ") + std::to_string(t->uid));
			t->flags |= r2::R2TileFlags::R2_TILE_IS_POOLED;
			if (break_Tile == t->uid)
				int breakOnAlloc = 0;
		};

		Pools::tiles.onFree = [](r2::Tile *t) {
			if (break_Tile == t->uid)
				int breakOnFree = 0;
			t->clear();
			t->flags &= ~r2::R2TileFlags::R2_TILE_IS_POOLED;
			//if( TRACK_POOLING ) trace(std::string("freeing ") + std::to_string(t->uid));
		};

		Pools::tiles.reset = [](r2::Tile *t) {
			t->clear();
		};

		static int break_Elem = 315;
		Pools::elems.reserve(32);
		Pools::elems.onFree = [](r2::BatchElem * b) {
			commonFree(b);
			int here = 0;
		};

		Pools::elems.onAlloc = [](r2::BatchElem * b) {
			commonAlloc(b);
			if (!b->tile) 
				b->setTile(Pools::tiles.alloc(),true);
		};

		Pools::aelems.reserve(32);
		Pools::aelems.onFree = [](rd::ABatchElem * b) {
			commonFree(b);
		};

		Pools::aelems.onAlloc = [](rd::ABatchElem * b) {
			commonAlloc(b);
		};

		Pools::subelems.onFree = [](rd::SubBatchElem * b) {
			commonFree(b);
		};

		Pools::subelems.onAlloc = [](rd::SubBatchElem * b) {
			commonAlloc(b);
		};

		Pools::subaelems.onFree = [](rd::SubABatchElem* b) {
			commonFree(b);
		};

		Pools::subaelems.onAlloc = [](rd::SubABatchElem* b) {
			commonAlloc(b);
		};

		Pools::bitmaps.reserve(32);
		Pools::bitmaps.onAlloc = [](r2::Bitmap * b) {
			commonAlloc(b);
			if (!b->tile) 
				b->setTile(rd::Pools::tiles.alloc(), true);
		};

		Pools::abitmaps.onAlloc = [](rd::ABitmap* b) {
			commonAlloc(b);
		};

		Pools::bitmaps.onFree = [](r2::Bitmap * b) {
			commonFree(b);
		};

		Pools::abitmaps.onFree = [](r2::Bitmap* b) {
			commonFree(b);
		};

		Pools::nodes.reserve(32);
		Pools::nodes.onAlloc = [](r2::Node * n) {
			commonAlloc(n);
		};

		Pools::nodes.onFree = [](r2::Node * n) {
			commonFree(n);
		};

		Pools::texts.reserve(32);
		Pools::texts.onFree = [](r2::Text * b) {
			commonFree(b);
		};

		Pools::texts.onAlloc = [](r2::Text * t) {
			commonAlloc(t);
			t->setFont(r2::GpuObjects::defaultFont);
		};

		Pools::graphics.onFree = [](r2::Graphics * b) {
			commonFree(b);
		};

		Pools::graphics.onAlloc = [](r2::Graphics * b) {
			commonAlloc(b);
		};

		Pools::batches.onAlloc = [](r2::Batch* b) {
			commonAlloc(b);
			b->name = std::string("Pooled Batch #") + std::to_string(b->uid);
		};

		Pools::batches.onFree = [](r2::Batch* b) {
			commonFree(b);
		};

		Pools::sbatches.onAlloc = [](rd::StaticBatch* b) {
			commonAlloc(b);
		};

		Pools::sbatches.onFree = [](rd::StaticBatch* b) {
			commonFree(b);
		};

		Pools::interacts.onAlloc = [](r2::Interact* b) {
			commonAlloc(b);
		};

		Pools::interacts.onFree = [](r2::Interact* b) {
			commonFree(b);
		};
	}
};

static _Init rdInit;

void rd::Pools::release(r2::BatchElem* el){
	if (el->beFlags & NF_IN_POOL)//already free drop it
		return;

	auto asASe = dynamic_cast<rd::SubABatchElem*>(el);
	if (asASe) {
		subaelems.release(asASe);
		return;
	}

	auto asSe = dynamic_cast<rd::SubBatchElem*>(el);
	if (asSe) {
		subelems.release(asSe);
		return;
	}

	auto asAe = dynamic_cast<rd::ABatchElem*>(el);
	if (asAe) {
		aelems.release(asAe);
		return;
	}

	elems.release(el);
}

void rd::Pools::release(rd::AnonAgent* ag) {
	aa.release(ag);
}

void rd::Pools::release(r2::Node* node) {
	if (! (node->nodeFlags & NF_ORIGINATES_FROM_POOL) ){
		int potentialError = 0;
#ifdef _DEBUG
		throw "sure?";
#endif
	}

	if (node->nodeFlags & NF_IN_POOL)//already free drop it
	{
		node->dispose();//at least make it gets dereferenced
		return;
	}

	auto asTxt = dynamic_cast<r2::Text*>(node);
	if (asTxt) {
		texts.release(asTxt);
		return;
	}

	auto asABmp = dynamic_cast<rd::ABitmap*>(node);
	if (asABmp) {
		abitmaps.release(asABmp);
		return;
	}

	auto asBmp = dynamic_cast<r2::Bitmap*>(node);
	if (asBmp) {
		bitmaps.release(asBmp);
		return;
	}

	auto asSBatch = dynamic_cast<rd::StaticBatch*>(node);
	if (asSBatch) {
		sbatches.release(asSBatch);
		return;
	}

	auto asBatch = dynamic_cast<r2::Batch*>(node);
	if (asBatch) {
		batches.release(asBatch);
		return;
	}

	auto asGfx = dynamic_cast<r2::Graphics*>(node);
	if (asGfx) {
		graphics.release(asGfx);
		return;
	}

	auto asInter = dynamic_cast<r2::Interact*>(node);
	if (asInter) {
		interacts.release(asInter);
		return;
	}

	nodes.release(node);
}


void rd::Pools::safeFree(r2::Node* node) {
	if (node->nodeFlags & NF_IN_POOL)//already free drop it
		return;

	auto asTxt = dynamic_cast<r2::Text*>(node);
	if (asTxt) {
		texts.safeFree(asTxt);
		return;
	}

	auto asABmp = dynamic_cast<rd::ABitmap*>(node);
	if (asABmp) {
		abitmaps.safeFree(asABmp);
		return;
	}
	auto asBmp = dynamic_cast<r2::Bitmap*>(node);
	if (asBmp) {
		bitmaps.safeFree(asBmp);
		return;
	}

	auto asSBatch = dynamic_cast<rd::StaticBatch*>(node);
	if (asSBatch) {
		sbatches.safeFree(asSBatch);
		return;
	}

	auto asBatch = dynamic_cast<r2::Batch*>(node);
	if (asBatch) {
		batches.safeFree(asBatch);
		return;
	}

	auto asGfx = dynamic_cast<r2::Graphics*>(node);
	if (asGfx) {
		graphics.safeFree(asGfx);
		return;
	}
	auto asInter = dynamic_cast<r2::Interact*>(node);
	if (asInter) {
		interacts.safeFree(asInter);
		return;
	}

	nodes.safeFree(asGfx);
}

Pasta::ShadedTexture* rd::Pools::allocForFrame()
{
	Pasta::ShadedTexture* st = imTexHolders.alloc();
	deleteList.push_back(st);
	return st;
}

void rd::Pools::scheduleForRawFree(void* membloc){
	rawFreeList.push_back(membloc);
}

void rd::Pools::exitFrame(){
	if (deleteList.size()) {
		for (Pasta::ShadedTexture* st: deleteList) 
			imTexHolders.release(st);
		deleteList.clear();
	}
	
	if (rawFreeList.size()) {
		for (void * blocks : rawFreeList)
			PASTA_FREE(blocks);
		rawFreeList.clear();
	}
}

r2::Text* rd::Pools::getText(const char * txt, rd::Font* fnt, int fontSize, r::Color col,r2::Node*parent ) {
	auto t = texts.alloc();
	t->setFont(fnt);
	if(fontSize)
		t->setFontSize(fontSize);
	t->setTextColor(col);
	t->setText(txt);
	if (parent)
		parent->addChild(t);
	return t;
}