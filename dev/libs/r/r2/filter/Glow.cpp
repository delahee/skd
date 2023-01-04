#include "stdafx.h"
#include "1-time/Profiler.h"
#include "Glow.hpp"
#include "rd/JSerialize.hpp"

using namespace std;
using namespace r2;
using namespace r2::filter;

#define  SUPER  r2::filter::Blur 
r2::filter::Glow::Glow() : SUPER(){
	bClear	= new Bitmap();
	bClear->releaseTile();

	bGlow	= new Bitmap();
	bGlow->releaseTile();
	type = FilterType::FT_GLOW;
}

r2::filter::Glow::Glow(const Vector2& size,
	const Color& color,
	float scale,
	r2::TexFilter filter,
	bool knockout ) : Glow() {
	set(size, color, scale, filter, knockout);
}

r2::filter::Glow::~Glow(){
	delete bClear; bClear = nullptr;
	delete bGlow; bGlow = nullptr;
}

void r2::filter::Glow::set(
	const r::Vector2 & size,
	const r::Color & color,
	float scale, r2::TexFilter filter, bool knockout){
	r2::filter::Blur::set(size, scale, filter);
	this->glowColor = color;
	this->knockout = knockout;
}

r2::Filter* r2::filter::Glow::clone(r2::Filter* _obj ) {
	Glow* obj = (Glow*)_obj;
	if (!obj) obj = new Glow();
	SUPER::clone(obj);
	obj->knockout = knockout;
	obj->matrixMode = matrixMode;
	obj->glowColor = glowColor;
	obj->colorMatrix= colorMatrix;
	return obj;
}

void r2::filter::Glow::serialize(Pasta::JReflect& jr, const char* name){
	SUPER::serialize(jr, name);
	jr.visit(knockout, "knockout");
	jr.visit(matrixMode, "matrixMode");
	jr.visit(glowColor, "glowColor");
	jr.visit(colorMatrix,"colorMatrix");
	jr.visit((int&)compositingFilter, "compositingFilter");
}

r2::Tile * r2::filter::Glow::filterTile(rs::GfxContext * g, r2::Tile * input){
	PASTA_CPU_GPU_AUTO_MARKER("Glow filterTile");

	g->push();
	r2::Tile * blurred = r2::filter::Blur::filterTile(g, input);

	if (forFiltering == nullptr) forFiltering = new RenderDoubleBuffer(compositingFilter);

	int pad = flattenPadding;
	int w = ceil(blurred->width);
	int h = ceil(blurred->height);

	RenderDoubleBuffer & rd = *forFiltering;

	rd.filter = compositingFilter;
	rd.update(w, h);

	if (h > 4096 || w > 4096) {
		int _break = 0;
	}

	Texture *		res = rd.getWorkingTexture();
	FrameBuffer *	resFb = rd.getWorkingFB();
	Scene *			sc = rd.sc;
	Tile*			dest = rd.getWorkingTile();
	
	Node *		n = rd::Pools::nodes.alloc();

	n->resetTRS();

	bClear->tile = forFlattening->getDrawingTile();
	bClear->ownsTile = false;
	bClear->x = -blurred->dx;
	bClear->y = -blurred->dy;
	bClear->blendmode = Pasta::TT_ALPHA;

	bGlow->tile = blurred;
	bGlow->setTRS(bClear->getTRS());

	sc->removeAllChildren();

	sc->addChild(n);

	Matrix44 mat;
	if (matrixMode) {
		mat = colorMatrix.transpose();
	}
	else {
		mat.setRow(0, Vector4(0, 0, 0, glowColor.r * glowIntensity * glowColor.a));
		mat.setRow(1, Vector4(0, 0, 0, glowColor.g * glowIntensity * glowColor.a));
		mat.setRow(2, Vector4(0, 0, 0, glowColor.b * glowIntensity * glowColor.a));
		mat.setRow(3, Vector4(0, 0, 0, glowColor.a));
	}

	bGlow->mkUber();
	bGlow->shaderFlags = UberShaderFlags::USF_ColorMatrix;
	bGlow->updateShaderParam("uColorMatrix", mat.ptr(), 16);
	bGlow->color = glowColorBlit;

	if (knockout) {
		bClear->blendmode = Pasta::TT_ERASE;
		bClear->color = (r::Color(5,5,5,5));
	}
	else {
		bClear->blendmode = Pasta::TT_ALPHA;
		bClear->color = (r::Color(1,1,1,1));
	}
	n->addChild(bGlow);
	n->addChild(bClear);

	n->drawTo(res, resFb, sc);

	Tile * d = rd.getWorkingTile();
	d->resetTargetFlip();
	d->setCenterDiscrete(blurred->dx, blurred->dy);
	d->setPos(0, 0);
	d->setSize(w, h);
	d->textureFlipY();

	bGlow->dispose();
	bClear->dispose();

	rd::Pools::nodes.free(n);
	g->pop();
	return rd.getDrawingTile();
}
