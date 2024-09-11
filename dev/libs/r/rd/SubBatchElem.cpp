#include "stdafx.h"
#include "SubBatchElem.hpp"

using namespace rd;

bool rd::SubBatchElem::poolServiceInstalled = false;
bool rd::SubABatchElem::poolServiceInstalled = false;

rd::SubBatchElem::SubBatchElem() : BatchElem() {
}

BeType rd::SubBatchElem::getType() const {
	return BeType::BET_SUB_BATCH_ELEM;
}

rd::SubBatchElem::~SubBatchElem() {
	dispose();
	if (tile)
		tile->destroy();
	tile = nullptr;
}

void rd::SubBatchElem::dispose() {
	parent = nullptr;
	Super::dispose();
}

void rd::SubBatchElem::reset() {
	Super::reset();
	parent = nullptr;
	ofs = {};
}

void rd::SubBatchElem::syncCoords() {
	if (!parent) 
		return;
	x = parent->x + ofs.x;
	y = parent->y + ofs.y;
	z = parent->z + ofs.z;
}

void rd::SubBatchElem::update(double dt) {
	Super::update(dt);
	syncCoords();
}

SubBatchElem * rd::SubBatchElem::fromPool(r2::Tile * tile, r2::BatchElem * parent, int priority) {
	auto elem = rd::Pools::subelems.alloc();
	elem->destroyed = false;
	elem->beFlags |= NF_ORIGINATES_FROM_POOL;
	elem->setTile(tile,true);
	elem->parent = parent;
	elem->priority = priority;
	if (!poolServiceInstalled) {
		rs::Svc::reg("SubBatchElem::pool::update", 
		[]() { 
			updatePool(rs::Timer::dt);
		});
		poolServiceInstalled = true;
	}
	return elem;
}

void rd::SubBatchElem::toPool() {
	dispose();
	rd::Pools::subelems.release(this);
}

void rd::SubBatchElem::updatePool(double dt) {
	int i = rd::Pools::subelems.active.size() - 1;
	for (; i >= 0; i--)
		rd::Pools::subelems.active[i]->update(dt);
}


double rd::SubBatchElem::getValue(rs::TVar valType) {
	switch (valType) {
		case VOfsX:	return ofs.x;
		case VOfsY:	return ofs.y;
		case VOfsZ:	return ofs.z;
		default:
			return Super::getValue(valType);
	}
}

double rd::SubBatchElem::setValue(rs::TVar valType, double val) {
	switch (valType) {
		case VOfsX:	ofs.x = val; syncCoords(); break; 
		case VOfsY:	ofs.y = val; syncCoords(); break;
		case VOfsZ:	ofs.z = val; syncCoords(); break;
		default:
			Super::setValue(valType, val);
			break;
	}
	return val;
}


rd::SubABatchElem::SubABatchElem() : ABatchElem() {
}

BeType rd::SubABatchElem::getType() const {
	return BeType::BET_SUB_ABATCH_ELEM;
}

rd::SubABatchElem::~SubABatchElem() {
	dispose();
	if (tile)
		tile->destroy();
	tile = nullptr;
}

void rd::SubABatchElem::dispose() {
	//traceObject("~disp", this);
	parent = nullptr;
	Super::dispose();
}

void rd::SubABatchElem::reset() {
	Super::reset();
	parent = nullptr;
	ofs = {};
}

void rd::SubABatchElem::syncCoords() {
	if (!parent)
		return;
	x = parent->x + ofs.x;
	y = parent->y + ofs.y;
	z = parent->z + ofs.z;
}

void rd::SubABatchElem::update(double dt) {
	syncCoords();

	Super::update(dt);
}

rd::SubABatchElem* rd::SubABatchElem::fromPool(rd::TileLib* l, const char* grp, r2::BatchElem* parent, int priority) {
	auto elem = rd::Pools::subaelems.alloc();
	elem->destroyed = false;
	elem->beFlags |= NF_ORIGINATES_FROM_POOL;
	elem->set(l);
	elem->playAndLoop(grp);
	elem->parent = parent;
	if (parent && parent->batch) {
		elem->priority = parent->priority - 1;
		parent->batch->add(elem);
	}
	elem->priority = priority;
	if (!poolServiceInstalled) {
		rs::Svc::reg("SubABatchElem::pool::update",
		[]() {
			updatePool(rs::Timer::dt);
		});
		poolServiceInstalled = true;
	}
	return elem;
}

void rd::SubABatchElem::toPool() {
	dispose();
	rd::Pools::subaelems.release(this);
}

void rd::SubABatchElem::updatePool(double dt) {
	int i = rd::Pools::subaelems.active.size() - 1;
	for (; i >= 0; i--)
		rd::Pools::subaelems.active[i]->update(dt);
}


double rd::SubABatchElem::getValue(rs::TVar valType) {
	switch (valType) {
	case VOfsX:	return ofs.x;
	case VOfsY:	return ofs.y;
	case VOfsZ:	return ofs.z;
	default:
		return Super::getValue(valType);
	}
}

double rd::SubABatchElem::setValue(rs::TVar valType, double val) {
	switch (valType) {
	case VOfsX:	ofs.x = val; syncCoords(); break;
	case VOfsY:	ofs.y = val; syncCoords(); break;
	case VOfsZ:	ofs.z = val; syncCoords(); break;
	default:
		Super::setValue(valType, val);
		break;
	}
	return val;
}
