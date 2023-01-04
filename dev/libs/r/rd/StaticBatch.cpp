#include "stdafx.h"

#include <vector>
#include <algorithm>

#include "r2/Sprite.hpp"
#include "r2/BatchElem.hpp"
#include "StaticBatch.hpp"
#include "rs/GfxContext.hpp"
#include <algorithm>

#include "platform.h"
#include "1-graphics/geo_vectors.h"
#include "1-graphics/Graphic.h"
#include "1-graphics/GraphicContext.h"
#include "1-graphics/GraphicEnums.h"
#include "1-graphics/ShaderProgram.h"
#include "1-graphics/ShaderParam.h"
#include "1-time/Profiler.h"

using namespace r2;
using namespace Pasta;

rd::StaticBatch::StaticBatch(r2::Node * parent) : r2::Batch::Batch(parent) {

}

void rd::StaticBatch::draw(rs::GfxContext * _g) {
	PASTA_CPU_GPU_AUTO_MARKER("Draw StaticBatch");
	if (nbElems == 0)
		return;

	if (!shouldRenderThisPass(_g)) return;

	Pasta::Graphic* g = _g->gfx;
	Pasta::GraphicContext * ctx = Pasta::GraphicContext::GetCurrent();

	_g->pushContext();

	if (trsDirty && !(nodeFlags & NF_MANUAL_MATRIX)) syncMatrix();
	_g->loadModelMatrix(mat);

	applyDepth(_g);
	setTransparency(_g, head);
	g->setVertexDeclaration(VD_POSITIONS | VD_TEXCOORDS | VD_COLORS);

	Pasta::Color c(color.r, color.g, color.b, color.a * alpha * _g->alpha.back());
	g->setColor(c);
	g->setAlpha(c.a);
	
	bindTile(_g, head->tile);
	bindShader(_g);

	const int stride = 3 + 2 + 4;
	if (!computed) {
		int count = nbElems;
		auto nbVisible = 0;
		for (auto it = head; it != nullptr; it = it->next) 
			if (it->shouldRender(_g))
				nbVisible++;
		const int verts = 6;
		fbuf.clear();
		fbuf.reserve(count * verts * stride);
		ibuf.clear();
		ibuf.reserve(count * 6);
		int curPos = 0;
		int i = 0;
		for (auto it = head; it != nullptr; it = it->next) {
			if (it->shouldRender(_g)) {
				curPos = i;
				i++;
				break;
			}
		}
		i = curPos;

		BatchElem * tHead = head;
		BatchElem * tI = nullptr;

		texture = tHead->getTexture();

		triangleCount = 0;

		tI = tHead;
		for (; i < count; i++) {
			if (!tI->shouldRender(_g)) {
				tI = tI->next;
				continue;
			}

			if (tI->getTile() == nullptr)
				assert(false); //et puis quoi encore

			if (tI->getTexture() == nullptr)
				assert(false); //???

			if (tI->getTexture() != texture )
				assert(false); //ahah nope hors de question

			tI->pushQuad(fbuf, ibuf);

			triangleCount += 2;
			tI = tI->next;
		}

		computed = true;
	}
	
#ifdef _DEBUG
	_g->nbBatchElemDrawn += triangleCount >>1;
	_g->nbDraw++;
#endif

	if (beforeDrawFlush) beforeDrawFlush();
	g->drawIndexed(PrimitiveType::Triangles, fbuf.data(), fbuf.size() / stride, ibuf.data(), ibuf.size());

	//post cleanup
	g->setVertexDeclaration(0);
	g->setTexture(ShaderStage::Fragment, 0, NULL);
	_g->popContext();
}

void rd::StaticBatch::add(r2::BatchElem * e) {
	invalidate();
	computed = false;
	Batch::add(e);
}

r2::BatchElem * rd::StaticBatch::alloc(r2::Tile * tile, double _priority) {
	invalidate();
	return Batch::alloc(tile, _priority);
}

void rd::StaticBatch::remove(r2::BatchElem * e) {
	invalidate();
	Batch::remove(e);
}

void rd::StaticBatch::free(r2::BatchElem * e) {
	invalidate();
	Batch::free(e);
}

void rd::StaticBatch::removeAllElements() {
	invalidate();
	Batch::removeAllElements();
}

void rd::StaticBatch::disposeAllElements() {
	invalidate();
	Batch::disposeAllElements();
}

void rd::StaticBatch::deleteAllElements() {
	invalidate();
	Batch::deleteAllElements();
}

void rd::StaticBatch::poolBackAllElements() {
	invalidate();
	Batch::poolBackAllElements();
}

void rd::StaticBatch::destroyAllElements() {
	invalidate();
	Batch::destroyAllElements();
}

void rd::StaticBatch::invalidate() {
	computed = false;
	triangleCount = 0;
	texture = nullptr;
}
