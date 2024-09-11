#include "stdafx.h"

#include "1-time/Profiler.h"
#include "4-ecs/JsonReflect.h"

#include "../Tile.hpp"
#include "../svc/Kernel.hpp"

#include "Blur.hpp"


using namespace r2;
using namespace r2::filter;
using namespace r2::svc;

Blur::Blur() : Super() {
	kH = new Kernel();
	kV = new Kernel();
	flattenPadding = 4;
	set(size, offsetScale, texFilter);
	type = FilterType::FT_BLUR;

}

r2::filter::Blur::Blur(const Vector2 & size, float scale, r2::TexFilter _filter) {
	kH = new Kernel();
	kV = new Kernel();
	texFilter = _filter;
	flattenPadding = 4;
	set(size, scale, texFilter);
	type = FilterType::FT_BLUR;
}

r2::filter::Blur::~Blur(){
	delete kH;
	delete kV;

	kH = nullptr;
	kV = nullptr;
}

void r2::filter::Blur::invalidate() {
	r2::Filter::invalidate();
	if (kH) kH->rd.isFresh = true;
	if (kV) kV->rd.isFresh = true;
}

void r2::filter::Blur::serialize(Pasta::JReflect& jr, const char* name) {
	if (name) jr.visitObjectBegin(name);
	Super::serialize(jr, 0);
	jr.visit(size, "size");
	jr.visit(offsetScale,"offsetScale");
	if (name) jr.visitObjectEnd(name);
}

r2::Filter* r2::filter::Blur::clone(r2::Filter* _obj) {
	Blur* obj = (Blur *)_obj;
	if (!obj) obj = new Blur();
	Super::clone(obj);
	obj->size.x = size.x;
	obj->size.y = size.y;
	obj->offsetScale = offsetScale;
	return obj;
}

void r2::filter::Blur::set(const Vector2 & _size, float scale, r2::TexFilter _filter){
	size.x = _size.x;
	size.y = _size.y;
	offsetScale = scale;
	texFilter = _filter;
	updateSize();
}

void r2::filter::Blur::updateSize() {
	int maxForPad = 0;
	if (size.x > maxForPad) maxForPad = ceil(Kernel::getKernelWidth(size.x));
	if (size.y > maxForPad) maxForPad = ceil(Kernel::getKernelWidth(size.y));
	flattenPadding = maxForPad * 1.5 * offsetScale * resolutionDivider;
}

r2::Tile *  Blur::filterTile(rs::GfxContext * g, r2::Tile * input){
	PASTA_CPU_GPU_AUTO_MARKER("Blur filterTile");
	int pad = flattenPadding;

    kH->rd.setSingleBufferMode(isSingleBuffer);
    kV->rd.setSingleBufferMode(isSingleBuffer);

	if (size.x <= 0 && size.y <= 0) {
		workingTile.copy(*input);
		return input;
	}

	//keep offsets consistent with frame
	r2::Tile* src = forFlattening->getWorkingTile();

	g->push();

	//only blur X
	if (size.y <= 0){
		kH->makeBlur1D(src->getTexture(), size.x, offsetScale, resolutionDivider, texFilter, true);
		
		r2::Tile* tH = kH->rd.getWorkingTile();
		tH->resetTargetFlip();
		tH->setCenterDiscrete(flatteningBounds.left() - pad, flatteningBounds.top() - pad);
        tH->resetTargetFlip();
        workingTile.copy(*tH);
        workingTile.width *= resolutionDivider; // total hack
		workingTile.height *= resolutionDivider;
		g->pop();

		return kH->rd.getDrawingTile();
	}
	//only blur X
	if (size.x <= 0) {
		kV->makeBlur1D(src->getTexture(), size.y, offsetScale, resolutionDivider, texFilter, false);
		
		r2::Tile* tV = kV->rd.getWorkingTile();
		tV->resetTargetFlip();
		tV->setCenterDiscrete(flatteningBounds.left() - pad, flatteningBounds.top() - pad);
        tV->resetTargetFlip();
        workingTile.copy(*tV);
        workingTile.width *= resolutionDivider; // total hack
		workingTile.height *= resolutionDivider;
		g->pop();

		return kV->rd.getDrawingTile();
	}
	
	kH->makeBlur1D(src->getTexture(), size.x, offsetScale, resolutionDivider, texFilter, true);
	r2::Tile* tH = kH->rd.getWorkingTile();	
	kV->makeBlur1D(tH->getTexture(), size.y, offsetScale, resolutionDivider, texFilter, false);
	r2::Tile* tV = kV->rd.getWorkingTile();
	tV->resetTargetFlip();
	tV->setCenterDiscrete(ceil(flatteningBounds.left() - pad), ceil(flatteningBounds.top() - pad));
	tV->resetTargetFlip();
    workingTile.copy(*tV);
	workingTile.width *= resolutionDivider; // total hack
	workingTile.height *= resolutionDivider;
	g->pop();

	return kV->rd.getDrawingTile();
}

#undef SUPER
