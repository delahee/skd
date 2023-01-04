#include "stdafx.h"

#include "Pools.hpp"

using namespace rd;
using namespace r2;

rs::Pool<TileAnim>						Pools::anims;
rs::Pool<r2::Tile>						Pools::tiles;
rs::Pool<rd::ABatchElem>				Pools::aelems;
rs::Pool<rd::SubBatchElem>				Pools::subelems;
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

eastl::vector<Pasta::ShadedTexture*>	Pools::deleteList;
eastl::vector<void*>					Pools::rawFreeList;

static void commonAlloc(r2::Node* b) {
	b->nodeFlags &= ~NF_IN_POOL;
	b->nodeFlags |= NF_ORIGINATES_FROM_POOL;
	b->reset();
}

static void commonAlloc(r2::BatchElem* b) {
	b->blendmode = Pasta::TT_INHERIT;
	b->beFlags &= ~NF_IN_POOL;
	b->beFlags |= NF_ORIGINATES_FROM_POOL;
}

static void commonFree(r2::Node* b) {
	b->nodeFlags |= NF_IN_POOL;
	b->dispose();
	b->resetTRS();
}

static void commonFree(r2::BatchElem* b) {
	b->clear();	
	b->dispose();
	b->beFlags |= NF_IN_POOL;
}

static void commonFree(r2::Sprite* b) {
	b->blendmode = TransparencyType::TT_ALPHA;
	b->nodeFlags |= NF_IN_POOL;
	b->dispose();
	b->resetTRS();
}

static void commonFree(r2::Graphics* b) {
	b->blendmode = TransparencyType::TT_ALPHA;
	b->nodeFlags |= NF_IN_POOL;
	b->dispose();
	b->resetTRS();
}


struct _Init {
	_Init() {
		Pools::anims.onFree = [](TileAnim * t) {
			t->clear();
		};

		static int break_Tile = 32;

		Pools::tiles.reserve(512);
		Pools::tiles.onAlloc = [](r2::Tile *t) {
			t->flags |= r2::R2TileFlags::R2_TILE_IS_POOLED;
			if (break_Tile == t->uid)
				int breakOnAlloc = 0;
		};

		Pools::tiles.onFree = [](r2::Tile *t) {
			if (break_Tile == t->uid)
				int breakOnFree = 0;
			t->clear();
			t->flags &= ~r2::R2TileFlags::R2_TILE_IS_POOLED;
			
		};

		Pools::tiles.reset = [](r2::Tile *t) {
			t->clear();
		};

		static int break_Elem = 315;
		Pools::elems.reserve(512);
		Pools::elems.onFree = [](r2::BatchElem * b) {
			commonFree(b);
		};

		Pools::elems.onAlloc = [](r2::BatchElem * b) {
			b->reset();
			b->setTile(Pools::tiles.alloc());
			b->ownsTile = true;
			commonAlloc(b);
		};

		Pools::aelems.reserve(128);
		Pools::aelems.onFree = [](rd::ABatchElem * b) {
			//if (break_Elem == b->uid)
			//	int breakOnFree = 0;
			commonFree(b);
		};

		Pools::aelems.onAlloc = [](rd::ABatchElem * b) {
			b->reset();
			commonAlloc(b);
		};

		Pools::subelems.onFree = [](rd::SubBatchElem * b) {
			commonFree(b);
		};

		Pools::subelems.onAlloc = [](rd::SubBatchElem * b) {
			b->reset();
			commonAlloc(b);
		};

		Pools::bitmaps.reserve(64);
		Pools::bitmaps.onAlloc = [](r2::Bitmap * b) {
			b->tile = rd::Pools::tiles.alloc();
			b->ownsTile = true;
			commonAlloc(b);
			b->reset();
		};

		Pools::abitmaps.onAlloc = [](rd::ABitmap* b) {
			commonAlloc(b);
			b->reset();
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

void rd::Pools::free(r2::BatchElem* el){
	if (el->beFlags & NF_IN_POOL)//already free drop it
		return;

	auto asAe = dynamic_cast<rd::ABatchElem*>(el);
	if (asAe) {
		aelems.free(asAe);
		return;
	}

	elems.free(el);
}

void rd::Pools::free(rd::AnonAgent* ag) {
	aa.free(ag);
}

void rd::Pools::free(r2::Node * node){
	if (node->nodeFlags & NF_IN_POOL)//already free drop it
	{
		node->dispose();//at least make it gets dereferenced
		return;
	}

	auto asTxt = dynamic_cast<r2::Text*>(node);
	if (asTxt) {
		texts.free(asTxt);
		return;
	}

	auto asABmp = dynamic_cast<rd::ABitmap*>(node);
	if (asABmp) {
		abitmaps.free(asABmp);
		return;
	}

	auto asBmp = dynamic_cast<r2::Bitmap*>(node);
	if (asBmp) {
		bitmaps.free(asBmp);
		return;
	}

	auto asSBatch = dynamic_cast<rd::StaticBatch*>(node);
	if (asSBatch) {
		sbatches.free(asSBatch);
		return;
	}

	auto asBatch = dynamic_cast<r2::Batch*>(node);
	if (asBatch) {
		batches.free(asBatch);
		return;
	}

	auto asGfx = dynamic_cast<r2::Graphics*>(node);
	if (asGfx) {
		graphics.free(asGfx);
		return;
	}

	auto asInter = dynamic_cast<r2::Interact*>(node);
	if (asInter) {
		interacts.free(asInter);
		return;
	}

	nodes.free(node);
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
		interacts.free(asInter);
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
			imTexHolders.free(st);
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