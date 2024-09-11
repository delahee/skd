#pragma once

#include "../rs/Pool.hpp"
#include "rd/TileAnim.hpp"
#include "rd/SubBatchElem.hpp"
#include "r2/Tile.hpp"
#include "r2/BatchElem.hpp"
#include "r2/Bitmap.hpp"
#include "r2/Text.hpp"
#include "r2/Interact.hpp"
#include "r2/Graphics.hpp"
#include "4-menus/imgui.h"




namespace rd{
	extern template class rs::Pool<TileAnim>;
	extern template class rs::Pool<r2::Tile>;
	extern template class rs::Pool<rd::ABatchElem>;
	extern template class rs::Pool<rd::ABitmap>;
	extern template class rs::Pool<rd::SubBatchElem>;
	extern template class rs::Pool<rd::SubABatchElem>;
	extern template class rs::Pool<r2::Bitmap>;
	extern template class rs::Pool<r2::Node>;
	extern template class rs::Pool<r2::Text>;
	extern template class rs::Pool<r2::Graphics>;
	extern template class rs::Pool<r2::Batch>;
	extern template class rs::Pool<rd::StaticBatch>;
	extern template class rs::Pool<Pasta::ShadedTexture>;
	extern template class rs::Pool<r2::BatchElem>;
	extern template class rs::Pool<rd::AnonAgent>;
	extern template class rs::Pool<r2::Interact>;
	extern template class rs::Pool<rd::Anon>;

class Pools {
public:
	

	static rs::Pool<TileAnim>				anims;
	static rs::Pool<r2::Tile>				tiles;
	static rs::Pool<rd::ABatchElem>			aelems;
	static rs::Pool<rd::ABitmap>			abitmaps;
	static rs::Pool<rd::SubBatchElem>		subelems;
	static rs::Pool<rd::SubABatchElem>		subaelems;
	static rs::Pool<r2::Bitmap>				bitmaps;
	static rs::Pool<r2::Node>				nodes;
	static rs::Pool<r2::Text>				texts;
	static rs::Pool<r2::Graphics>			graphics;
	static rs::Pool<r2::Batch>				batches;
	static rs::Pool<rd::StaticBatch>		sbatches;
	static rs::Pool<Pasta::ShadedTexture>	imTexHolders;
	static rs::Pool<r2::BatchElem>			elems;
	static rs::Pool<rd::AnonAgent>			aa;
	static rs::Pool<r2::Interact>			interacts;
	static rs::Pool<rd::Anon>				anons;
	
	//dispatching free
	static void release(r2::BatchElem* nodes);
	static void release(r2::Node* nodes);
	static void release(rd::AnonAgent* ag);
	static void safeFree(r2::Node* nodes);
	
	static Pasta::ShadedTexture*		allocForFrame();
	static void							scheduleForFree(Pasta::ShadedTexture * text);

	//prefer calloc, use it only if you really know what you do
	static void							scheduleForRawFree( void * membloc );

	static void							exitFrame();

	
	static r2::Text*					getText(const char* txt,rd::Font* fnt,int fontSize = 0, r::Color col = r::Color(1,1,1,1),r2::Node*parent=0);
	static inline r2::Text* getText(const std::string& txt, rd::Font* fnt, int fontSize = 0, r::Color col = r::Color(1, 1, 1, 1), r2::Node* parent = 0) {
		return getText(txt.c_str(), fnt, fontSize, col, parent);
	};
	static inline r2::Text* getText(const Str& txt, rd::Font* fnt, int fontSize = 0, r::Color col = r::Color(1, 1, 1, 1), r2::Node* parent = 0) {
		return getText(txt.c_str(), fnt, fontSize, col, parent);
	};

	static eastl::vector<Pasta::ShadedTexture*>		deleteList;
	static eastl::vector<void*>						rawFreeList;
};
}