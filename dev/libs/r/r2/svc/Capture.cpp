#include "stdafx.h"

#include "1-graphics/FrameBuffer.h"
#include "1-graphics/Graphic.h"
#include "1-time/Profiler.h"
#include "1-graphics/FrameBuffer.h"
#include "../Lib.hpp"
#include "../Node.hpp"

#include "Capture.hpp"

using namespace std;
using namespace r2;
using namespace r2::svc;

static Tile * testTile = new r2::Tile();
bool Capture::recordTile;
eastl::vector<r2::Tile>	Capture::debugTiles;

Capture::Capture() {
	
}

r2::Tile * Capture::make(r::Texture * t, const vector<BmpOp> & bmpOps, r2::TexFilter filter , int pad) {
	stub.mapTexture(t);
	return make(&stub,bmpOps,filter,pad);
}

r2::Tile * r2::svc::Capture::make(r2::Tile * srcTile, const vector<BmpOp>& bmpOps, r2::TexFilter filter, int pad){
	if (!srcTile) {
		dummy.copy(*r2::GpuObjects::whiteTile);
		return &dummy;
	}

	pad += additionnalPad;

	int w = ceil(srcTile->width + pad * 2);
	int h = ceil(srcTile->height + pad * 2);

	int szTexW = w;
	int szTexH = h;
	rd.isSingleBuffer = isSingleBuffer;
	
	if(useStableTextureSize ){
		szTexW = ceil((w / texPadForStability) + 1) * texPadForStability;//pad to nearest 128
		szTexH = ceil((h / texPadForStability) + 1) * texPadForStability;//pad to nearest 128
		rd.update(szTexW, szTexH);
	}
	else {
		rd.update(w, h);
	}

	Texture *		tex		= rd.getWorkingTexture();
	FrameBuffer *	resFb	= rd.getWorkingFB();
	Scene *			sc		= rd.sc;
	Bitmap *		b		= rd.bmp;

	if (debugClearColor) {
		sc->doClear = true;
		sc->clearColor = debugClearColorVal;
	}

	b->detach();
	b->x = -srcTile->dx + pad;
	b->y = -srcTile->dy + pad;
	b->setToLocalMatrix();
	
	b->copyTile(srcTile);

	sc->resetTRS();
	sc->removeAllChildren();

	for(BmpOp bmpOp : bmpOps)
		bmpOp(*b);

	b->drawTo(tex, resFb, sc);

	r2::Tile * d = rd.getWorkingTile();

	d->resetTargetFlip(); 
	d->setCenterDiscrete(srcTile->dx - pad, srcTile->dy - pad);
	d->setPos(0, 0);
	d->setSize(w, h);
	d->textureFlipY();

	if (recordTile) {
		if (debugTiles.size() < 6) {
			debugTiles.push_back(*rd.getDrawingTile());
		}
	}
	return rd.getDrawingTile();
}

r2::Tile * Capture::make(r2::Node * n, r2::TexFilter filter, int pad){
	if (!n)return nullptr;

	pad += additionnalPad;

	TRS origin = n->getTRS();
	n->resetTRS();
	n->setToLocalMatrix();
	n->syncChildrenMatrix();
		
		r2::Bounds bnd = n->getMeasures(true);
		debugBounds = bnd;

		int fw = bnd.width() + pad * 2;
		int fh = bnd.height() + pad * 2;

		int w = ceil(bnd.width() + pad * 2);
		int h = ceil(bnd.height() + pad * 2);

		int szTexW = w;
		int szTexH = h;
		rd.isSingleBuffer = isSingleBuffer;
		if (useStableTextureSize) {
			szTexW = ((w / texPadForStability) + 1) * texPadForStability;//pad to nearest 128
			szTexH = ((h / texPadForStability) + 1) * texPadForStability;//pad to nearest 128
			rd.update(szTexW, szTexH);
		}
		else {
			rd.update(w, h);
		}

		Texture *		res = rd.getWorkingTexture();
		FrameBuffer *	resFb = rd.getWorkingFB();
		Scene *			sc = rd.sc;
		Bitmap *		b = rd.bmp;

		if (debugClearColor) {
			sc->doClear = true;
			sc->clearColor = debugClearColorVal;
		}

		//stabilize uv jitter
		//warning will probably cause deresolution 
		//should offer possibility to deactivate?
		float left = ceil(-bnd.left() + pad);
		float top = ceil(-bnd.top() + pad);
		
		n->x = left;
		n->y = top;
		n->setToLocalMatrix();
		n->syncChildrenMatrix();

		n->drawTo(res, resFb, sc);

		Tile * d = rd.getWorkingTile();
		d->resetTargetFlip();
		d->setCenterDiscrete(-left + pad, -top + pad);
		d->setPos(0,0);
		d->setSize(w, h);
		d->textureFlipY();

	sc->removeAllChildren();
	n->setTRS(origin);

	return rd.getDrawingTile();
}

void r2::svc::Capture::im()
{
	ImGui::PushID(this);
	if (ImGui::CollapsingHeader("Capture")) {
		
		ImGui::Indent();
		ImGui::LabelText("inst", "0x%lx", uintptr_t(this));
		ImGui::Checkbox("useStableTextureSize", &useStableTextureSize);
		ImGui::Checkbox("isSingleBuffer", &isSingleBuffer);
		ImGui::SliderInt("texPadForStability", &texPadForStability, 1, 1024);
		ImGui::SliderInt("additionnalPad;", &additionnalPad, 0, 128);

		ImGui::Checkbox("debug clear color", &debugClearColor);
		if (debugClearColor)
			ImGui::ColorEdit4("debug color val", debugClearColorVal.ptr());

		if (ImGui::CollapsingHeader("Rd")) {
			ImGui::Indent();
			rd.im();
			ImGui::Unindent();
		}
		
		ImGui::Unindent();
	}
	ImGui::PopID();
}



