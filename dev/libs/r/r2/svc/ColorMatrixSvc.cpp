#include "stdafx.h"

#include "1-time/Profiler.h"

#include "r2/svc/ColorMatrix.hpp"

using namespace std;

static string uColorMatrix = string("uColorMatrix");

r2::svc::ColorMatrix::ColorMatrix() {
	
}

static r2::svc::ColorMatrix * cm = nullptr;

r2::svc::ColorMatrix & r2::svc::ColorMatrix::get() {
	if (cm == nullptr)
		cm = new r2::svc::ColorMatrix();
	return *cm;
}

r2::Tile* r2::svc::ColorMatrix::make(r2::Tile * src, const r2::ColorMatrixControl& ctrl) {
	if (ctrl.mode == ColorMatrixMode::CMM_HSV && ctrl.hue == 0.0f && ctrl.sat == 1.0f && ctrl.val == 1.0f)
		return src;

	//todo filter only the requested portion in a rt
	return make(src->getTexture(), ctrl.mat, ctrl.filter);
}

r2::Tile* r2::svc::ColorMatrix::make(
	r::Texture* tex,
	const Pasta::Matrix44 & ctrl,
	r2::TexFilter filter) {
	PASTA_CPU_GPU_AUTO_MARKER("texColorMatrix");

	if (!tex) return nullptr;
	
	if (ctrl == r::Matrix44::identity) {
		c.dummy.mapTexture(tex);
		c.rd.isFresh = true;
		return &c.dummy;
	}
	
	ops.clear();
	ops.push_back([=](r2::Bitmap&b) {
		b.mkUber();
		b.shaderFlags |= r2::UberShaderFlags::USF_ColorMatrix;
		b.updateShaderParam(uColorMatrix, ctrl.ptr(),16);
	});

	return c.make(tex, ops, filter);
}