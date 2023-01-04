#include "stdafx.h"
#include "Scissor.hpp"
#include "Bounds.hpp"
#include "rs/GfxContext.hpp"
#include "1-graphics/Graphic.h"
#include "1-device/DeviceMgr.h"

#define SUPER Node

r2::Scissor::Scissor(Node* parent) : SUPER(parent){
	rect = r2::Rect(0,0,rs::Display::width(), rs::Display::height());
}

r2::Scissor::Scissor(Rect r, Node * parent) : SUPER(parent){
	rect = r;
}

void r2::Scissor::drawRec(rs::GfxContext * _g) {
	Pasta::DeviceMgr * d = Pasta::DeviceMgr::getSingleton();
	Pasta::Graphic * g = _g->gfx;

	_g->pushScissor();

	if (isRelative) {
		syncMatrix();

		Pasta::Vector3 tl(rect.x, rect.y, 0.0f);
		Pasta::Vector3 br(rect.x + rect.width, rect.y + rect.height, 0.0f);

		Pasta::Vector3 ttl = mat * tl;
		Pasta::Vector3 tbr = mat * br;

		int px = (int)ttl.x;
		if (px >= tbr.x) px = tbr.x;
		int py = (int)ttl.y;
		if (py >= tbr.y) py = tbr.y; //in case scale is neg and has warped stuff

		int w = (int)std::rint(tbr.x - ttl.x);
		int h = (int)std::rint(tbr.y - ttl.y);

		_g->setScissor(px, py, w, h);
	}
	else {
		int px = (int)rect.x;
		int py = (int)rect.y;
		int w = (int)std::rint(rect.width);
		int h = (int)std::rint(rect.height);

		_g->setScissor(px, py, w, h);
	}

	SUPER::drawRec(_g);

	_g->popScissor();
}

/**
* not tested enough...
*/
r2::Bounds r2::Scissor::getMyLocalBounds()
{
	Bounds b = SUPER::getMyLocalBounds();

	if (isRelative) {
		syncMatrix();

		Pasta::Vector3 tl(rect.x, rect.y, 0.0f);
		Pasta::Vector3 br(rect.x + rect.width, rect.y + rect.height, 0.0f);

		auto local = getLocalMatrix();
		Pasta::Vector3 ttl = local * tl;
		Pasta::Vector3 tbr = local * br;

		int px = (int)ttl.x;
		if (px >= tbr.x) px = tbr.x;
		int py = (int)ttl.y;
		if (py >= tbr.y) py = tbr.y; //in case scale is neg and has warped stuff

		int w = (int)std::rint(tbr.x - ttl.x);
		int h = (int)std::rint(tbr.y - ttl.y);

		if (b.xMin < px ) b.xMin = px;
		if (b.yMin < py ) b.yMin = py;

		if (b.xMax > px) b.xMax = px + w;
		if (b.yMax > py) b.yMax = py + h;
	}
	else {
		//does it really mean anything at this stage
		// crush all umans!
		return Bounds::fromTLWH(rect.x, rect.y, rect.width, rect.height);
	}

	return b;
}

#undef SUPER