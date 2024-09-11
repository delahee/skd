#include "stdafx.h"

#include "1-graphics/FrameBuffer.h"
#include "1-graphics/Graphic.h"
#include "1-time/Profiler.h"
#include "1-graphics/FrameBuffer.h"

#include "r2/RenderDoubleBuffer.hpp"

#include "Glitch.hpp"
#include "Capture.hpp"

#include "../Lib.hpp"

using namespace Pasta;
using namespace std;
using namespace r2;
using namespace r2::svc;

Glitch::Glitch() {
}

static Glitch * glitch = nullptr;

Glitch & Glitch::get()
{
	if (glitch == nullptr) {
		glitch = new Glitch();
	}
	return *glitch;
}

r2::Tile * r2::svc::Glitch::make(r2::Node * src, const GlitchControl & ctrl)
{
	if (!src) return nullptr;

	if (!cCapture) cCapture = new Capture();

	std::vector<BmpOp> nop;
	cCapture->make(src, ctrl.filter, pad);
	return make(cCapture->getWorkingTile(), ctrl);
}

r2::Tile * r2::svc::Glitch::make(r2::Tile * src, const GlitchControl & ctrl)
{
	PASTA_CPU_GPU_AUTO_MARKER("texGlitch");

	if (!src) return nullptr;

	bool proceed = false;

	if (ctrl.glitchAmount.x != 0.0)
		proceed = true;

	if (!cFilter) cFilter = new Capture();

	if (!proceed) {
		cFilter->dummy.copy(*src);
		cFilter->rd.isFresh = true;
		return &cFilter->dummy;
	}

	ops.clear();
	ops.push_back([=](r2::Bitmap&b) {//am not even ashamed
		b.mkUber();
		b.shaderFlags |= USF_Glitch;
		b.updateShaderParam("uGlitchParams", ctrl.glitchAmount.ptr(), 4);
	});

	return cFilter->make(src, ops, ctrl.filter);
}

r2::Tile* Glitch::make(
	r::Texture* tex,
	const GlitchControl & ctrl) {
	PASTA_CPU_GPU_AUTO_MARKER("texGlitch");

	bool proceed = false;

	if (ctrl.glitchAmount.x != 0.0)
		proceed = true;

	if (!cFilter) cFilter = new Capture();

	if (!proceed) {
		cFilter->dummy.mapTexture(tex);
		cFilter->rd.isFresh = true;
		return &cFilter->dummy;
	}

	ops.clear();
	ops.push_back([=](r2::Bitmap&b) {//am not even ashamed
		b.mkUber();
		b.shaderFlags |= USF_Glitch;
		b.updateShaderParam("uGlitchParams", ctrl.glitchAmount.ptr(), 4);
	});

	return cFilter->make(tex, ops, ctrl.filter);
}

void r2::svc::Glitch::im()
{
	if (cCapture) {
		ImGui::Text("capture");
		cCapture->im();
	}
	if (cFilter) {
		ImGui::Text("filter");
		cFilter->im();
	}
}
