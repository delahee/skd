#include "stdafx.h"

#include "1-time/Profiler.h"
#include "Filter.hpp"
#include "RenderDoubleBuffer.hpp"

#include "rd/JSerialize.hpp"

using namespace r;
using namespace r2;

Filter::Filter() { }

Filter::~Filter(){
	if(forFiltering)
		delete forFiltering; 
	forFiltering = nullptr;
	if(forFlattening)
		delete forFlattening; 
	forFlattening = nullptr;
}

r2::Bounds Filter::getMyLocalBounds() {	
	return flatteningBounds;
}

static void setMatrixToLocal(r2::Node * n) {
	n->overrideMatrix(n->getLocalMatrix());
}

r2::Filter* r2::Filter::clone(r2::Filter* obj) {
	if (!obj) obj = new r2::Filter();
	obj->enabled = enabled;
	obj->mode = mode;
	obj->type = type;
	obj->texFilter = texFilter;
	return obj;
}

void r2::Filter::serialize(Pasta::JReflect& jr, const char* name) {
	if (name) jr.visitObjectBegin(name);
	jr.visit(enabled, "enabled");
	jr.visit((int&)mode, "mode");
	jr.visit((int&)type, "type");
	jr.visit((int&)texFilter, "texFilter");
	if (name) jr.visitObjectEnd(name);
}

r2::Tile* r2::Filter::captureHierarchy(rs::GfxContext * _g, r2::Node * n, int pad){
	//hits marker limit
	PASTA_CPU_GPU_AUTO_MARKER("r2::Filter::captureHierarchy");
	Scene* sc = n->getScene();
	Pasta::Graphic * g = _g->gfx;
	Pasta::GraphicContext * ctx = Pasta::GraphicContext::GetCurrent();
	if (forFlattening == nullptr)
		forFlattening = new RenderDoubleBuffer(texFilter);

	TRS basis = n->getTRS();
	n->resetTRS();
	Matrix44 oldMat;
	n->copyMatrix(oldMat);
	n->overrideMatrix(Matrix44::identity);
	n->syncChildrenMatrix();

		r2::Bounds b = flatteningBounds = n->getMeasures(true);
		int iLeft = ceil(b.left());
		int iTop = ceil(b.top());

		double ow = b.width()  + double(pad) * 2;
		double oh = b.height() + double(pad) * 2;

		int w = ceil(ow * (includeScale ? basis.scaleXscaleYRotation.x : 1.0f));
		int h = ceil(oh * (includeScale ? basis.scaleXscaleYRotation.y : 1.0f));

		int szTexW = w;
		int szTexH = h;

		forFlattening->filter = texFilter;
		if (useStableTextureSize) {
			szTexW = ceil((w / texPadForStability) + 1) * texPadForStability;//pad to nearest 128
			szTexH = ceil((h / texPadForStability) + 1) * texPadForStability;//pad to nearest 128
			forFlattening->update(szTexW, szTexH);
		}
		else 
			forFlattening->update(w, h);
		
	
		Texture * t = forFlattening->getWorkingTexture();
		FrameBuffer * fb = forFlattening->getWorkingFB();

		n->x = -iLeft;
		n->y = -iTop;
		n->x += pad;
		n->y += pad;
		if (includeScale) {
			n->scaleX = basis.scaleXscaleYRotation.x;
			n->scaleY = basis.scaleXscaleYRotation.y;
			n->x *= basis.scaleXscaleYRotation.x;
			n->y *= basis.scaleXscaleYRotation.y;
		}
		n->setToLocalMatrix();
		n->syncChildrenMatrix();

		int lw = t->getLogicalWidth(); 
		int lh = t->getLogicalHeight();

		_g->push();
			
				_g->pushTarget(fb);
		
					_g->pushScissor();
					_g->setScissor(0, 0, lw, lh);

					//forFlattening->sc->setDepthRange(sc->getDepthRange());
					forFlattening->sc->setDepthRange(1024);
						_g->defaultZ();
						forFlattening->sc->stdMatrix(_g,lw, lh);

						_g->clear(	PASTA_CLEAR_COLOR | PASTA_CLEAR_DEPTH,
									debugUseClearColor ? debugClearColor : r::Color(0.0, 0, 0, 0.0),
									_g->zMax);

						n->drawContent(_g);

						cdebugProj = _g->projMatrix;
						cdebugModel = _g->modelMatrix;
						cdebugView = _g->viewMatrix;

					_g->popScissor();
				_g->popTarget();
			
		_g->pop();

	n->restoreMatrix(oldMat);
	n->setTRS(basis);
	//n->syncChildrenMatrix();

	Tile * work = forFlattening->getWorkingTile();//stay on this frame data to work
	work->resetTargetFlip();
	work->setCenterDiscrete(iLeft - pad, iTop - pad);
	work->setPos(0, 0);
	work->setSize(lw, lh);
	work->textureFlipY();
	
	//let swap chain decide what to send back
	return forFlattening->getDrawingTile();
}


Tile* Filter::filterTile(rs::GfxContext * __g, r2::Tile * src){
	PASTA_CPU_GPU_AUTO_MARKER("r2::Filter::filterTile");
	rs::GfxContext & _g = *__g;
	Pasta::GraphicContext * ctx = _g.getGpuContext();
	Pasta::Graphic * g = _g.gfx;

	if (forFiltering == nullptr)
		forFiltering = new RenderDoubleBuffer(texFilter);

	int w = ceil(src->width);
	int h = ceil(src->height);

	//could tile stabilizer here but it is generally ok because tile stabilize will probably be done at capture stage

	forFiltering->filter = texFilter;
	forFiltering->update(w, h);

	Scene * sc = forFiltering->sc;
	sc->removeAllChildren();
	sc->resetTRS();

	Bitmap * bmp = forFiltering->bmp;
	Tile * dest = forFiltering->getWorkingTile();
	FrameBuffer * fb = forFiltering->getWorkingFB();
	Pasta::Texture * t = forFiltering->getWorkingTexture();
	
	bmp->tile = src;
	bmp->ownsTile = false;

	int lw = t->getLogicalWidth();
	int lh = t->getLogicalHeight();

	int iTop = ceil(flatteningBounds.top());
	int iLeft = ceil(flatteningBounds.left());
	sc->addChild(bmp);
	bmp->blendmode = Pasta::TT_ALPHA;
	bmp->resetTRS();

	bmp->x = -iLeft;
	bmp->y = -iTop;
	
	bmp->mkClassic();//restore the std shader, let the bmpOps do their magic

	for (BmpOp & op : bmpOps)
		op(*bmp);

	_g.push();
		
				_g.defaultZ();
				forFiltering->sc->stdMatrix(&_g, lw, lh);

					_g.pushScissor();
					_g.setScissor(0, 0, lw, lh);


						if (debugUseClearColor)
							sc->clearColor = debugClearColor;
						else 
							sc->clearColor = r::Color(0, 0.0, 0, 0.0);
						sc->doClear = true;
						sc->drawInto(&_g, sc, t, fb);

						fdebugProj = _g.projMatrix;
						fdebugModel = _g.modelMatrix;
						fdebugView = _g.viewMatrix;

					_g.popScissor();

		
	_g.pop();

	sc->removeAllChildren();

	bmp->tile = nullptr;

	Tile * d = forFiltering->getWorkingTile();
	d->resetTargetFlip();
	d->setCenterDiscrete(src->dx, src->dy);
	d->setPos(0, 0);
	d->setSize(lw, lh);
	d->textureFlipY();
	
	Tile* res = forFiltering->getDrawingTile();
	res->getTexture()->setWrapModeUVW(PASTA_TEXTURE_CLAMP);
	return res;
}

void r2::Filter::pushBitmapOp(BmpOp op){
	bmpOps.push_back(op);
}

void r2::Filter::popBitmapOp(){
	bmpOps.pop_back();
}

bool r2::Filter::shouldComputeFilter(){
	switch (mode) {
		case r2::FilterMode::FM_Frozen:
			if (nbRender >= 2)
				return false;
			break;

		default:
			break;
	}
	return true;
}

void r2::Filter::compute(rs::GfxContext* g, r2::Node* n) {
	//hits marker limit
	//PASTA_CPU_GPU_AUTO_MARKER("r2::Filter::compute");
	curNode = n;

	if(onComputeStart) onComputeStart();

	g->push();

	//could get drawing tile here
	r2::Tile* tCapture = captureHierarchy(g, n, flattenPadding);
	if (!tCapture) {
		g->pop();
		return;
	}

	//otherwise keep using working tile for filtering
	r2::Tile* tFiltered = filterTile(g, forFlattening->getWorkingTile());
	if (!tFiltered) {
		g->pop();
		return;
	}
	result = tFiltered;

	if (includeScale) {
		result->width /= n->scaleX;
		result->height /= n->scaleY;
	}
	nbRender++;
	g->pop();
}

r2::Tile * r2::Filter::getResult(){
	return result;
}

void r2::Filter::invalidate(){
	nbRender = 0;
}
