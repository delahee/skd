#include "stdafx.h"

#include "SubBatchElem.hpp"

rd::SubBatchElem::SubBatchElem() : BatchElem() {
}

rd::SubBatchElem::~SubBatchElem() {
	dispose();
	if (tile)
		tile->destroy();
	tile = nullptr;
}

void rd::SubBatchElem::dispose() {
	BatchElem::dispose();
	parent = nullptr;
}

void rd::SubBatchElem::reset() {
	BatchElem::reset();

	parent = nullptr;

	offX = 0;
	offY = 0;
}

void rd::SubBatchElem::update(double dt) {
	if (!parent) return;
	x = parent->x + offX;
	y = parent->y + offY;
}

void rd::SubBatchElem::setOff(double x, double y) {
	offX = x;
	offY = y;
}

SubBatchElem * rd::SubBatchElem::fromPool(r2::Tile * tile, r2::BatchElem * parent, int priority) {
	auto elem = rd::Pools::subelems.alloc();
	elem->setTile(tile);
	elem->parent = parent;
	elem->priority = priority;
	return elem;
}

void rd::SubBatchElem::toPool() {
	dispose();
	rd::Pools::subelems.free(this);
}

void rd::SubBatchElem::updatePool(double dt) {
	int i = rd::Pools::subelems.active.size() - 1;
	for (; i >= 0; i--)
		rd::Pools::subelems.active[i]->update(dt);
}
