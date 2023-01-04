#include "stdafx.h"
#include "RenderDoubleBuffer.hpp"

using namespace r2;

r2::RenderDoubleBuffer::RenderDoubleBuffer(r2::TexFilter filter) {
	sc = new Scene();
	sc->removeEventManagement();
	sc->doClear = true;
	sc->clearColor = r::Color(0, 0, 0, 0);
	bmp = new Bitmap();
	this->filter = filter;
}

r2::RenderDoubleBuffer::~RenderDoubleBuffer() {
	if (sc) {
		delete sc;
		sc = nullptr;
	}
	if (bmp) {
		delete  bmp;
		bmp = nullptr;
	}
	if (workBuff) {
		SurfacePool::get()->freeSfb(workBuff);
		workBuff = nullptr;
	}
	if (backingBuff) {
		SurfacePool::get()->freeSfb(backingBuff);
		backingBuff = nullptr;
	}
}

//returns true if surface was resized
void r2::RenderDoubleBuffer::update(int w, int h) {
	int reqW = w;
	int reqH = h;

	if (reqW < 1) reqW = 1;
	if (reqH < 1) reqH = 1;

	if (!isSingleBuffer) {

		bool flush = false;

		if (isFresh) {
			flush = true;
		}

		if (!flush) {
			if (!backingBuff) {
				backingBuff = workBuff;
				workBuff = nullptr;
			}
			else {
				auto finishedBuff = workBuff;//n-2
				workBuff = backingBuff;
				backingBuff = finishedBuff;
			}

			if (backingBuff) {
				backingTile.copy(workingTile);
				backingTile.setTexture(backingBuff->getRt());
			}
		}
		else {
			backingBuff = workBuff;
			if (backingBuff) {
				backingTile.copy(workingTile);
				backingTile.setTexture(backingBuff->getRt());
			}
			workingTile.setTexture(nullptr);
			if (workBuff) {
				SurfacePool::get()->freeSfb(workBuff);
				if (backingBuff == workBuff)//hu?
					backingBuff = nullptr;
			}
			workBuff = nullptr;
		}

		workingTile.clear();
		
		side = (!workBuff) ? CurrentSide::Front : CurrentSide::Back;
	}
	else {
		if (backingBuff) {
			SurfacePool::get()->freeSfb(backingBuff);
			backingBuff = nullptr;
		}
		backingTile.clear();
		side = CurrentSide::Front;
		//workbuff may or maybe not null
	}

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
	workingTile.mapTexture(workBuff->getRt());
}

r2::Tile* r2::RenderDoubleBuffer::getWorkingTile() {
	return &workingTile;
}

r2::Tile* r2::RenderDoubleBuffer::getDrawingTile() {
	return (side == CurrentSide::Back) ? &backingTile : &workingTile;
}

Pasta::Texture* r2::RenderDoubleBuffer::getDrawingTexture() {
	return getDrawingTile()->getTexture();
}

Pasta::Texture* r2::RenderDoubleBuffer::getWorkingTexture() {
	if (!workBuff) return nullptr;
	return workBuff->getRt();
}

Pasta::FrameBuffer* r2::RenderDoubleBuffer::getDrawingFB() {
	if (side == CurrentSide::Front && !workBuff) return nullptr;
	if (side == CurrentSide::Back && !backingBuff) return nullptr;
	return (side == CurrentSide::Back) ? backingBuff->buffer : workBuff->buffer;
}

Pasta::FrameBuffer* r2::RenderDoubleBuffer::getWorkingFB() {
	if (!workBuff) return nullptr;
	return workBuff->buffer;
}

void r2::RenderDoubleBuffer::im()
{
	ImGui::PushID(this);
	ImGui::Checkbox("isSingleBuffer", &isSingleBuffer);
	ImGui::Checkbox("imDisplayfull", &imDisplayfull);
	ImGui::Text("Current side : %s", (side == CurrentSide::Back)?"back":"front");

	if(!im0) im0 = new Pasta::ShadedTexture();
	if(!im1) im1 = new Pasta::ShadedTexture();
	

	if (isSingleBuffer) {
		ImGui::Text("SingleBuffer");
		r2::Tile& tile = workingTile;
		auto tex = tile.getTexture();
		if (tex) {
			im0->texture = tex;
			ImGui::LabelText("size.x", "%d", tex->getWidth());
			ImGui::LabelText("size.y", "%d", tex->getHeight());
			float ratio = 1.0 * tex->getWidth() / tex->getHeight();

			int dispHeight = imDisplayfull ? tex->getHeight() : 100;
			ImGui::Image((ImTextureID)im0, ImVec2(dispHeight * ratio, dispHeight), ImVec2(0, 1), ImVec2(1, 0));

			tile.im();
		}
	}
	else {
		ImGui::Text("DoubleBuffer");

		ImGui::Columns(2);
		if (workingTile.getTexture())
		{
			auto tex = workingTile.getTexture();
			ImGui::Text("Working %dx%d", tex->getWidth(), tex->getHeight());
			im0->texture = tex;
			float ratio = 1.0 * tex->getWidth() / tex->getHeight();

			int dispHeight = imDisplayfull ? tex->getHeight() : 100;
			ImGui::Image((ImTextureID)im0, ImVec2(dispHeight * ratio, dispHeight), ImVec2(0, 1), ImVec2(1, 0));
			workingTile.im();
		}

		ImGui::NextColumn();
		if(backingTile.getTexture())
		{
			auto tex = backingTile.getTexture();
			ImGui::Text("Backing %dx%d", tex->getWidth(), tex->getHeight());
			im1->texture = tex;
			float ratio = 1.0 * tex->getWidth() / tex->getHeight();
			int dispHeight = imDisplayfull ? tex->getHeight() : 100;
			ImGui::Image((ImTextureID)im1, ImVec2(dispHeight * ratio, dispHeight), ImVec2(0, 1), ImVec2(1, 0));
			backingTile.im();
		}
		ImGui::Columns(1);
	}
	ImGui::PopID();

}
