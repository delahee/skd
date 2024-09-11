#include "stdafx.h"
#include "RenderDoubleBuffer.hpp"

using namespace r2;

r2::RenderDoubleBuffer::RenderDoubleBuffer(r2::TexFilter filter, bool isSingleBuffer) : isSingleBuffer(isSingleBuffer) {
	sc = new Scene();
	sc->removeEventManagement();
	sc->doClear = true;
	sc->clearColor = r::Color(0, 0, 0, 0);
	bmp = new Bitmap();
	this->filter = filter;
}

r2::RenderDoubleBuffer::~RenderDoubleBuffer() {
	if (sc) {
		sc->destroy();
		sc = nullptr;
	}
	if (bmp) {
		bmp->destroy();
		bmp = nullptr;
	}
	if (workBuff) {
		SurfacePool::get()->freeSfb(workBuff);
		workBuff = nullptr;
	}
	if (frontBuff) {
		SurfacePool::get()->freeSfb(frontBuff);
		frontBuff = nullptr;
	}
}

//returns true if surface was resized
void r2::RenderDoubleBuffer::update(int w, int h) {
	int reqW = w;
	int reqH = h;

	if (reqW < 1) reqW = 1;
	if (reqH < 1) reqH = 1;

    bool flush = isSingleBuffer; // constantly flush in single-buffer Mode
    if (isFresh) {
        flush = true;
        isFresh = false;
    }

	if (!isSingleBuffer) {
        // we swap the front & back buffer so that the back buffer is shown to the screen & the previous front is rewritten
        auto finishedBuff = workBuff;
        workBuff = frontBuff;
        frontBuff = finishedBuff;
        if (frontBuff) {
            frontTile.copy(workTile);
            frontTile.setTexture(frontBuff->getRt());
        }
    }

    if (flush) {
        workTile.setTexture(nullptr);
        if (workBuff)
            SurfacePool::get()->freeSfb(workBuff);
        workBuff = nullptr;
    }

    workTile.clear();

	SingleFbPage * fb = workBuff;

	if (fb && (fb->w < reqW || fb->h < reqH || fb->filter != filter || fb->colorFormat != colorFormat)) {
		SurfacePool::get()->freeSfb(fb);
		fb = workBuff = nullptr;
	}

	if (!fb) {
		fb = workBuff = SurfacePool::get()->allocSingleFb(reqW, reqH, filter, true, colorFormat);
	}

	if (!fb)
		printf("ERR : no surface\n");

	//after this workbuf
	workTile.mapTexture(workBuff->getRt());
}

void RenderDoubleBuffer::setSingleBufferMode(bool mode) {
	if (isSingleBuffer == mode) return;
    isSingleBuffer = mode;

    isFresh = true;
    workTile.setTexture(nullptr);
    frontTile.setTexture(nullptr);

    if (frontBuff)
		SurfacePool::get()->freeSfb(frontBuff);
    frontBuff = nullptr;
}

r2::Tile* r2::RenderDoubleBuffer::getWorkingTile() {
	return &workTile;
}

r2::Tile* r2::RenderDoubleBuffer::getDrawingTile() {
    //return isSingleBuffer ? &workTile : &frontTile;
    if (isSingleBuffer) return &workTile;
    return frontBuff ? &frontTile : &workTile;
}

Pasta::Texture* r2::RenderDoubleBuffer::getWorkingTexture() {
    if (!workBuff) return nullptr;
    return workBuff->getRt();
}

Pasta::Texture* r2::RenderDoubleBuffer::getDrawingTexture() {
    //if (!frontBuff) return nullptr;
    if (isSingleBuffer) return workBuff->getRt();
    return frontBuff ? frontBuff->getRt() : workBuff->getRt();
}

Pasta::FrameBuffer* r2::RenderDoubleBuffer::getWorkingFB() {
    if (!workBuff) return nullptr;
    return workBuff->buffer;
}

Pasta::FrameBuffer* r2::RenderDoubleBuffer::getDrawingFB() {
    //if (!frontBuff) return nullptr;
	if (isSingleBuffer) return workBuff->buffer;
	return frontBuff ? frontBuff->buffer : workBuff->buffer;
}

void r2::RenderDoubleBuffer::im() {
	ImGui::PushID(this);
	bool single = isSingleBuffer;
    if(ImGui::Checkbox("isSingleBuffer", &single))
		setSingleBufferMode(single);
	ImGui::Checkbox("imDisplayfull", &imDisplayfull);

	if(!im0) im0 = new Pasta::ShadedTexture();
	if(!im1) im1 = new Pasta::ShadedTexture();
	

	if (isSingleBuffer) {
		ImGui::Text("SingleBuffer");
		r2::Tile& tile = workTile;
		auto tex = tile.getTexture();
        if (tex) {
            ImGui::Text("Buffer %dx%d", tex->getWidth(), tex->getHeight());
			im0->texture = tex;
			float ratio = 1.0 * tex->getWidth() / tex->getHeight();

			int dispHeight = imDisplayfull ? tex->getHeight() : 100;
			ImGui::Image((ImTextureID)im0, ImVec2(dispHeight * ratio, dispHeight), ImVec2(0, 1), ImVec2(1, 0));

			tile.im();
		}
	}
	else {
		ImGui::Text("DoubleBuffer");

		ImGui::Columns(2);
		if (workTile.getTexture())
		{
			auto tex = workTile.getTexture();
			ImGui::Text("Work %dx%d", tex->getWidth(), tex->getHeight());
			im0->texture = tex;
			float ratio = 1.0 * tex->getWidth() / tex->getHeight();

			int dispHeight = imDisplayfull ? tex->getHeight() : 100;
			ImGui::Image((ImTextureID)im0, ImVec2(dispHeight * ratio, dispHeight), ImVec2(0, 1), ImVec2(1, 0));
			workTile.im();
		}

		ImGui::NextColumn();
		if (frontTile.getTexture())
		{
			auto tex = frontTile.getTexture();
			ImGui::Text("Front %dx%d", tex->getWidth(), tex->getHeight());
			im1->texture = tex;
			float ratio = 1.0 * tex->getWidth() / tex->getHeight();
			int dispHeight = imDisplayfull ? tex->getHeight() : 100;
			ImGui::Image((ImTextureID)im1, ImVec2(dispHeight * ratio, dispHeight), ImVec2(0, 1), ImVec2(1, 0));
			frontTile.im();
		}
		ImGui::Columns(1);
	}
	ImGui::PopID();

}
