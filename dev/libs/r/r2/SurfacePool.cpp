#include "stdafx.h"
#include "SurfacePool.hpp"

using namespace r2;
using namespace Pasta;
using namespace eastl;

/* DoubleFbPage */
Pasta::Texture* r2::DoubleFbPage::getRt() {
	return frontBuffer->getColorAttachments()->m_texture;
}

r2::DoubleFbPage::~DoubleFbPage() {
	if (frontBuffer) r2::Lib::destroyFramebuffer(frontBuffer);
	if (backBuffer) r2::Lib::destroyFramebuffer(backBuffer);
	
	frontBuffer = backBuffer = nullptr;
}

void r2::DoubleFbPage::swapBuffer() {
	auto fb = frontBuffer;
	frontBuffer = backBuffer;
	backBuffer = fb;

	SurfacePool* sp = SurfacePool::get();
	sp->curTime = rs::Timer::frameCount;
	dirtyTime = sp->curTime;
}

/* SingleFbPage */
Pasta::Texture* r2::SingleFbPage::getRt() {
	return buffer->getColorAttachments()->m_texture;
}

r2::SingleFbPage::~SingleFbPage() {
	if (buffer) r2::Lib::destroyFramebuffer(buffer);

	buffer = nullptr;
}

/* SurfacePool */
static SurfacePool * sp = nullptr;
SurfacePool::SurfacePool(){}

r2::SurfacePool::~SurfacePool(){
	for (auto iter = sfbs.begin(); iter != sfbs.end(); ) {
		delete (*iter);
		iter++;
	}

	for (auto iter = dfbs.begin(); iter != dfbs.end(); ) {
		delete (*iter);
		iter++;
	}
}

SurfacePool* SurfacePool::get(){
	if (!sp) sp = new SurfacePool();
	return sp;
}

r2::DoubleFbPage* SurfacePool::allocDoubleFb(int w, int h, r2::TexFilter filter) {
	curTime = rs::Timer::frameCount;

	if (enabled) {
		for (auto iter = dfbs.begin(); iter != dfbs.end(); ) {
			int64_t deltaTime = curTime - (*iter)->dirtyTime;

			if (deltaTime >= 2) { //it's okay to give it back
				auto* p = *iter;
				if (p->w == w && p->h == h && p->filter == filter) {
					iter = dfbs.erase(iter);
					p->dirtyTime = curTime;
					return p;
				}
			}
			iter++;
		}
	}

	DoubleFbPage* npage = new DoubleFbPage();
	npage->frontBuffer = r2::Lib::createRenderTextureFB(w, h, filter, nullptr, true, false);
	npage->backBuffer = r2::Lib::createRenderTextureFB(w, h, filter, nullptr, true, false);
	npage->dirtyTime = curTime;
	npage->w = w;
	npage->h = h;
	npage->filter = filter;
	return npage;
}

r2::SingleFbPage* SurfacePool::allocSingleFb(int w, int h, r2::TexFilter filter, bool depth, Pasta::TextureFormat::Enum colorFormat) {
	curTime = rs::Timer::frameCount;

	if (enabled) {
		for (auto iter = sfbs.begin(); iter != sfbs.end(); ) {
			int64_t deltaTime = curTime - (*iter)->dirtyTime;

			if (deltaTime >= 2) { //it's okay to give it back
				auto* p = *iter;
				bool hasDepth = p->buffer->getDepthAttachment();
				if (p->w == w && p->h == h && p->filter == filter && hasDepth == depth && p->colorFormat == colorFormat) {
					iter = sfbs.erase(iter);
					p->dirtyTime = curTime;
					return p;
				}
			}
			iter++;
		}
	}

	SingleFbPage* npage = new SingleFbPage();
	npage->buffer = r2::Lib::createRenderTextureFB(w, h, filter, nullptr, true, depth, colorFormat);
	npage->dirtyTime = curTime;
	npage->w = w;
	npage->h = h;
	npage->filter = filter;
	npage->colorFormat = colorFormat;
	return npage;
}


void SurfacePool::freeDfb(DoubleFbPage * p ){
	curTime = rs::Timer::frameCount;
	if (!enabled) {
		delete(p);
		return;
	}
	if (p == nullptr) return;
	p->dirtyTime = curTime;
#if _DEBUG
	if (rs::Std::exists(dfbs, p)) {
		int i = 0; // double entry, not ok!
	}
#endif
	dfbs.push_back(p);
}

void SurfacePool::freeSfb(SingleFbPage * p) {
	curTime = rs::Timer::frameCount;
	if (!enabled) {
		delete(p);
		return;
	}
	if (p == nullptr) return;
	p->dirtyTime = curTime;
#if _DEBUG
	if (rs::Std::exists(sfbs, p)) {
		int i = 0; // double entry, not ok!
	}
#endif
	sfbs.push_back(p);
}


void r2::SurfacePool::update() {
	curTime = rs::Timer::frameCount;
	for (auto iter = sfbs.begin(); iter != sfbs.end(); ) {
		Page * pg = *iter;
		int64_t deltaTime = curTime - pg->dirtyTime;
		if (deltaTime >= DELETE_SURFACE_AGE) {
			delete(pg);
			iter = sfbs.erase(iter);
		}
		else 
			iter++;
	}

	for (auto iter = dfbs.begin(); iter != dfbs.end(); ) {
		Page * pg = *iter;
		int64_t deltaTime = curTime - pg->dirtyTime;
		if (deltaTime >= DELETE_SURFACE_AGE) {
			delete(pg);
			iter = dfbs.erase(iter);
		}
		else
			iter++;
	}
}


void r2::SurfacePool::im() {
	if (ImGui::TreeNode("r2::SurfacePool")) {
		ImGui::Text("Repo Single Framebuffer: %lu", sfbs.size());
		ImGui::Text("Repo Double Framebuffer: %lu", dfbs.size());
		ImGui::TreePop();
	}
}
