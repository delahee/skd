#include "stdafx.h"

#include "../Tile.hpp"
#include "r2/Filter.hpp"
#include "Layer.hpp"
#include "Bloom.hpp"
#include "../svc/Bloom.hpp"
#include "../../rd/JSerialize.hpp"
#include "1-time/Profiler.h"

using namespace r2;
using namespace r2::filter;

#define SUPER r2::filter::Layer
r2::filter::Bloom::Bloom() : r2::filter::Layer() {
	ctrl.doPad = true;
	type = FilterType::FT_BLOOM;
}

void r2::filter::Bloom::serialize(Pasta::JReflect& jr, const char* name){
	if (name) jr.visitObjectBegin(name);
	SUPER::serialize(jr, 0);
	jr.visit(ctrl.dx, "dx");
	jr.visit(ctrl.dy, "dy");
	jr.visit(ctrl.luminanceVector, "lum");
	jr.visit(ctrl.pass, "pass");
	jr.visit(ctrl.skipBloomPass, "skipBloomPass");
	jr.visit(ctrl.skipColorPass, "skipColorPass");
	jr.visit(ctrl.doPad, "doPad");
	jr.visit((int&)ctrl.blurFilter, "blurFilter");
	jr.visit((int&)ctrl.renderFilter, "renderFilter");
	jr.visit(ctrl.pass, "pass");
	jr.visit(ctrl.offsetScale, "offsetScale");
	jr.visit(ctrl.mul, "mul");
	jr.visit(ctrl.intensity, "intensity");
	if (name) jr.visitObjectEnd(name);
};

r2::Filter* r2::filter::Bloom::clone(r2::Filter* _obj) {
	Bloom* obj = (Bloom*)_obj;
	if (!obj) obj = new Bloom();
	SUPER::clone(obj);
	obj->bloom.ctrl = bloom.ctrl;
	return obj;
}

r2::filter::Bloom::Bloom(const BloomCtrl& _ctrl) {
	this->ctrl = _ctrl;
	type = FilterType::FT_BLOOM;
}

r2::filter::Bloom::~Bloom(){
}

void r2::filter::Bloom::set(const BloomCtrl& _ctrl) {
	this->ctrl = _ctrl;
}

r2::Tile* Bloom::filterTile(rs::GfxContext * g, r2::Tile * input) {
	PASTA_CPU_GPU_AUTO_MARKER("Bloom filterTile");
	bloom.ctrl = ctrl;
	bloom.make(input);
	workingTile.copy(bloom.workingTile);
	return &bloom.drawingTile;
}
