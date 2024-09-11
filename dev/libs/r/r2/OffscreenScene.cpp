#include "stdafx.h"

#include "1-time/Profiler.h"
#include "r2/OffscreenScene.hpp"
#include "r2/Scene.hpp"

using namespace r2;
using namespace Pasta;

#define SUPER r2::Scene

OffscreenScene::OffscreenScene(int w, int h, r2::TexFilter filter) : wantedWidth(w), wantedHeight(h), rd(filter, isSingleBuffer) {
	targetRatioW = 1.0;
	targetRatioH = 1.0;

	clearColor = r::Color(0, 0, 0, 0);
}

OffscreenScene::~OffscreenScene() {
	
}

void OffscreenScene::render(Pasta::Graphic * g){
	PASTA_CPU_GPU_AUTO_MARKER("Render OffscreenScene");
	rs::GfxContext _g = rs::GfxContext(g);
	GraphicContext * ctx = GraphicContext::GetCurrent();

	rd.setSingleBufferMode(isSingleBuffer);
	rd.update(wantedWidth, wantedHeight);

	FrameBuffer * _fb = rd.getWorkingFB();
	Texture * t = rd.getWorkingTexture();

	if (!_fb || !t) return;

	_g.push();
		_g.pushTarget(_fb);
		_g.pushScissor();

		_g.setScissor(0, 0, wantedWidth, wantedHeight);
		if (doClear) _g.clear(PASTA_CLEAR_COLOR | PASTA_CLEAR_DEPTH, clearColor, _g.zMax );
		stdMatrix(&_g, t->getLogicalWidth(), t->getLogicalHeight());
		drawRec(&_g);

		_g.popScissor();
		_g.popTarget();

	_g.pop();
	
	dest.copy(*rd.getDrawingTile());
}

Pasta::Texture* r2::OffscreenScene::getDepthTexture() {
	auto fb = rd.getDrawingFB();
	return fb->getDepthAttachment()->m_texture;
}

r2::Tile * r2::OffscreenScene::getTargetTile() {
	return rd.getDrawingTile();
}

#undef SUPER