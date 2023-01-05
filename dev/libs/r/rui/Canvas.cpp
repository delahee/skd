#include "stdafx.h"
#include "Canvas.hpp"


using namespace rui;

//ex 640x360 => 1440 x 900
static void topLeftContain( const r::Vector2 & srcSize, const r::Vector2 & destSize, r2::Node* src) {
	float ratioSrcDst = destSize.y / srcSize.y;
	src->x = 0;
	src->y = 0;
	src->scaleY = ratioSrcDst;
	src->scaleX = src->scaleY;
}

void Canvas::resetContent() {
	resetTRS();
	if (enabled) {
		onResize(rs::Display::getSize());
	}
}

void Canvas::runAndRegResize(std::function<void(void)> f) {
	f();
	sigOnResize.add(f);
}

r::Vector2 Canvas::getMousePos()
{
	return r::Vector2(rs::Sys::mouseX / scaleX, rs::Sys::mouseY / scaleY);
}

Vector2 Canvas::getRefSize() {
	Vector2 refSize(-1, -1);

	//no edm
	if (refSize.x == -1 && refSize.y == -1) {
		refSize.x = fallbackRefSize.x;
		refSize.y = fallbackRefSize.y;
	}
	else {//desperate are we
		refSize = getSize();
	}
	return refSize;
}

float Canvas::getScaleX() { 
	return scaleX; 
}

float Canvas::getScaleY() { 
	return scaleY; 
}

Vector2 Canvas::getActualSize()
{
	float actualW = rs::Display::width();
	float actualH = rs::Display::height();

	float scX = getScaleX();
	float scY = getScaleY();

	return Vector2(
		actualW / scX,
		actualH / scY
	);
}

void Canvas::onRemoveChild(r2::Node* n) {
	
}

Canvas::Canvas(r2::Node* parent) : r2::Node(parent) {
	onResize(rs::Display::getSize());
}

Canvas::~Canvas() {
	dispose();
}

void Canvas::dispose(){
	for (auto& p : resizeHolder)
		sigOnResize.remove(p.second);
	resizeHolder.clear();
	r2::Node::dispose();
}

float Canvas::getPixelRatio() const {
	return scaleY;
}

r2::Node* Canvas::dock(r2::Node* spr, r::DIRECTION dir, float ofs) {
	rui::dock(spr, this, dir, ofs);
	resizeHolder[spr] = sigOnResize.add([=]() {
		rui::dock(spr, this, dir, ofs);
	});
	spr->onDestruction.addOnce([=]() {
		auto iter = resizeHolder.find(spr);
		if (iter != resizeHolder.end()) {
			sigOnResize.remove((*iter).second);
			resizeHolder.erase(iter);
		}
	});
	return spr;
}

r2::Node* Canvas::dock(r2::Node* spr, r::DIRECTION dir, Vector2 ofs) {
	rui::dock(spr, this, dir, ofs);
	resizeHolder[spr] = sigOnResize.add([=]() {
		rui::dock(spr, this, dir, ofs);
		});
	spr->onDestruction.addOnce([=]() {
		auto iter = resizeHolder.find(spr);
		if (iter != resizeHolder.end()) {
			sigOnResize.remove((*iter).second);
			resizeHolder.erase(iter);
		}
	});
	return spr;
}

r2::Node* Canvas::dockOnce(r2::Node* spr, r::DIRECTION dir, Vector2 ofs) {
	rui::dock(spr, this, dir, ofs);
	return spr;
}

void Canvas::onResize(const Vector2& ns) {
	if (!enabled)
		return;

	Vector2 refSize = getRefSize();
	topLeftContain(Vector2(refSize.x, refSize.y), rs::Display::getSize(), this);

	sigOnResize.trigger();
	r2::Node::onResize(ns);
}
