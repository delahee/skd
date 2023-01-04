#include "stdafx.h"

#include "Batch.hpp"
#include "BatchElem.hpp"
#include "Tile.hpp"
#include "rd/JSerialize.hpp"

using namespace r;
using namespace r2;

BatchElem::BatchElem() {
	uid = rs::Sys::getUID();
	ownsTile = false;
}

BatchElem::BatchElem(Tile * _tile, Batch * parent, double _priority) {
	uid = rs::Sys::getUID();
	tile = _tile;
	ownsTile = !tile;
	if (ownsTile) tile = rd::Pools::tiles.alloc();
	priority = _priority;
	if (parent) parent->add(this);
}

r2::BatchElem::BatchElem(const BatchElem & src){
	name = src.name;
	x = src.x;
	y = src.y;
	z = src.z;

	scaleX		= src.scaleX;
	scaleY		= src.scaleY;
	rotation	= src.rotation;

	alpha = src.alpha;
	color = src.color;

	priority = src.priority;
	visible = src.visible;

	blendmode = src.blendmode;
	setTile(src.tile->clone());
	ownsTile = true;

	//don't pass batch infos
	batch = nullptr;
}

BatchElem::~BatchElem() {
	if (beFlags & NF_ORIGINATES_FROM_POOL) {
		int breakOnWrongUse = 0;
	}
	dispose();
	batch = nullptr;
	next = nullptr;
	prev = nullptr;
}

//do not clean states we can be projected back into another batch
void BatchElem::remove() {
	if (batch) {
		batch->remove(this);
	}
	batch = nullptr;
}

void BatchElem::dispose() {
	remove();
	setTile(nullptr);
	destroyed = true;
	visible = false;
}

void r2::BatchElem::destroy(){
	dispose();
	if(beFlags & NF_CUSTOM_POOLING ) {
		//
	}
	else if (beFlags & NF_SKIP_DESTRUCTION ) {
		//
	}
	else if (beFlags & NF_ORIGINATES_FROM_POOL) {
		rd::Pools::free(this);
	}
	else {
		delete this;
	}
}

//bool BatchElemTest = false;

void r2::BatchElem::pushQuad(eastl::vector<float>& vertices, eastl::vector<u32>& indices) {
	Tile* tile = getTile();
	Matrix44 local;
	local.setScale(scaleX, scaleY, 1.0f);
	if (rotation) local.rotateZ(PASTA_RAD2DEG(rotation));
	local.translate(Pasta::Vector3(x, y, z));

	const float zTop = useSimpleZ ? 0.0f : zTopOffset;
	const float zBottom = useSimpleZ ? 0.0f : zBottomOffset;

	Vector3 topLeft(local * Vector3(tile->dx, tile->dy, zTop));
	Vector3 topRight(local * Vector3(tile->dx + tile->width, tile->dy, zTop));
	Vector3 bottomLeft(local * Vector3(tile->dx, tile->dy + tile->height, zBottom));
	Vector3 bottomRight(local * Vector3(tile->dx + tile->width, tile->dy + tile->height, zBottom));

	/*
	if (BatchElemTest) {
		float bend = 12;
		float h = bottomLeft.y - topLeft.y;
		float coef = h / 30.0f;
		topLeft.x += bend * coef;
		topRight.x += bend * coef;
	}
	*/

	const float u1 = tile->u1;
	const float v1 = tile->v1;
	const float u2 = tile->u2;
	const float v2 = tile->v2;
	const float r = color.r;
	const float g = color.g;
	const float b = color.b;
	const float a = color.a * alpha;

	const u32 offsetIndex = vertices.size() / 9;

#define ADD(v)				vertices.push_back(v);
#define ADD_POS(x,y,z)		ADD(x); ADD(y); ADD(z);
#define ADD_UV(u,v)			ADD(u); ADD(v);
#define ADD_RGBA(r,g,b,a)	ADD(r); ADD(g); ADD(b); ADD(a);

	ADD_POS(topLeft.x, topLeft.y, topLeft.z);
	ADD_UV(u1, v1);
	ADD_RGBA(r, g, b, a);

	ADD_POS(topRight.x, topRight.y, topRight.z);
	ADD_UV(u2, v1);
	ADD_RGBA(r, g, b, a);

	ADD_POS(bottomLeft.x, bottomLeft.y, bottomLeft.z);
	ADD_UV(u1, v2);
	ADD_RGBA(r, g, b, a);

	ADD_POS(bottomRight.x, bottomRight.y, bottomRight.z);
	ADD_UV(u2, v2);
	ADD_RGBA(r, g, b, a);

#define ADD_INDEX(v)		indices.push_back(v + offsetIndex);
#define ADD_TRI(a,b,c)		ADD_INDEX(a); ADD_INDEX(b); ADD_INDEX(c);

	ADD_TRI(0, 1, 2);
	ADD_TRI(2, 1, 3);
}

void r2::BatchElem::pushQuadNormals(eastl::vector<float>& vertices, eastl::vector<u32>& indices) {
	Tile* tile = getTile();
	Matrix44 local;
	local.setScale(scaleX, scaleY, 1.0f);
	if (rotation) local.rotateZ(PASTA_RAD2DEG(rotation));
	local.translate(Pasta::Vector3(x, y, z));

	const float zTop = useSimpleZ ? 0.0f : zTopOffset;
	const float zBottom = useSimpleZ ? 0.0f : zBottomOffset;

	Vector3 topLeft(local * Vector3(tile->dx, tile->dy, zTop));
	Vector3 topRight(local * Vector3(tile->dx + tile->width, tile->dy, zTop));
	Vector3 bottomLeft(local * Vector3(tile->dx, tile->dy + tile->height, zBottom));
	Vector3 bottomRight(local * Vector3(tile->dx + tile->width, tile->dy + tile->height, zBottom));

	const float u1 = tile->u1;
	const float v1 = tile->v1;
	const float u2 = tile->u2;
	const float v2 = tile->v2;
	const float r = color.r;
	const float g = color.g;
	const float b = color.b;
	const float a = color.a * alpha;

	const u32 offsetIndex = vertices.size() / 12;

#define ADD(v)				vertices.push_back(v);
#define ADD_POS(x,y,z)		ADD(x); ADD(y); ADD(z);
#define ADD_UV(u,v)			ADD(u); ADD(v);
#define ADD_NORMALS()		ADD(-1); ADD(0); ADD(0);
#define ADD_RGBA(r,g,b,a)	ADD(r); ADD(g); ADD(b); ADD(a);

	ADD_POS(topLeft.x, topLeft.y, topLeft.z);
	ADD_UV(u1, v1);
	ADD_NORMALS();
	ADD_RGBA(r, g, b, a);

	ADD_POS(topRight.x, topRight.y, topRight.z);
	ADD_UV(u2, v1);
	ADD_NORMALS();
	ADD_RGBA(r, g, b, a);

	ADD_POS(bottomLeft.x, bottomLeft.y, bottomLeft.z);
	ADD_UV(u1, v2);
	ADD_NORMALS();
	ADD_RGBA(r, g, b, a);

	ADD_POS(bottomRight.x, bottomRight.y, bottomRight.z);
	ADD_UV(u2, v2);
	ADD_NORMALS();
	ADD_RGBA(r, g, b, a);

#define ADD_INDEX(v)		indices.push_back(v + offsetIndex);
#define ADD_TRI(a,b,c)		ADD_INDEX(a); ADD_INDEX(b); ADD_INDEX(c);

	ADD_TRI(0, 1, 2);
	ADD_TRI(2, 1, 3);
}

double BatchElem::width() { return scaleX * tile->width; }
double BatchElem::height() { return scaleY * tile->height; }

void BatchElem::setWidth(double w) {
	if (tile->width == 0) { scaleX = 0; return; }
	scaleX = w / tile->width;
}

void BatchElem::setHeight(double h) {
	if (tile->height == 0) { scaleY = 0; return; }
	scaleY = h / tile->height;
}

void BatchElem::setSize(double w, double h) {
	setWidth(w);
	setHeight(h);
}

void r2::BatchElem::setPriority(double p) {
	if (batch) {
		r2::Batch * b = batch;
		if (false) {//old slow but safe way
			priority = p;
			batch->remove(this);
			b->add(this);
		}
		else {
			batch->changePriority(this, p);
			batch->sanityCheck();
		}
	}
	else
		priority = p;
}

void r2::BatchElem::reset() {
	clear();
	destroyed = false;
}

void r2::BatchElem::clear() {
	if (batch != nullptr) batch->remove(this);
	x = 0.0;
	y = 0.0;
	z = 0.0;

	scaleX		= 1.0f;
	scaleY		= 1.0f;
	rotation	= 0.0;

	alpha		= 1.0f;
	color.set();

	priority	= 0;
	visible		= true;

	blendmode	= Pasta::TransparencyType::TT_INHERIT;

	setTile(nullptr);

	//don't pass batch infos
	batch		= nullptr;
}

double BatchElem::getValue(TVar valType) {
	switch (valType) {
		case VX:		return x;
		case VY:		return y;
		case VZ:		return z;
		case VScaleX:	return scaleX;
		case VScaleY:	return scaleY;
		case VRotation: return rotation;
		case VScale:	return scaleX * scaleY;
		case VWidth:	return width();
		case VHeight:	return height();

		case VR:		return color.r;
		case VG:		return color.g;
		case VB:		return color.b;
		case VA:		return color.a;
		case VAlpha:	return alpha;
	}
	return 0.0;
}

double BatchElem::setValue(TVar valType, double val) {
	switch (valType) {
		case VX:		x		= val;				break;
		case VY:		y		= val;				break;
		case VZ:		z		= val;				break;
		case VScaleX:	scaleX	= val;				break;
		case VScaleY:	scaleY	= val;				break;
		case VRotation: rotation = val;				break;
		case VScale:	scaleX	= scaleY = val;		break;
		case VWidth:	setWidth(val);				break;
		case VHeight:	setHeight(val);				break;

		case VR:		color.r = val;				break;
		case VG:		color.g = val;				break;
		case VB:		color.b = val;				break;
		case VA:		color.a = val;				break;
		case VAlpha:	alpha = val;				break;
	}
	return val;
}

void r2::BatchElem::setTile(r2::Tile * t){
	if (tile && ownsTile) {
		tile->destroy();
		tile = nullptr;
	}

	//no own no touch get rid of this
	tile = t;
	ownsTile = false;
}

// Override base tile pivot! (can cause problem with real frame)
void BatchElem::setCenterRatio(double px, double py) {
	if(tile) tile->setCenterRatio(px, py);
}

r2::BatchElem* BatchElem::clone(r2::BatchElem * nu) const {
	if (nu == nullptr) 
		nu = rd::Pools::elems.alloc();
	nu->name = name;
	nu->x = x;
	nu->y = y;
	nu->z = z;

	nu->scaleX = scaleX;
	nu->scaleY = scaleY;
	nu->rotation = rotation;

	nu->alpha = alpha;
	nu->color = color;

	nu->priority = priority;
	nu->visible = visible;

	nu->blendmode = blendmode;

	nu->setTile(tile->clone());

	nu->useSimpleZ = useSimpleZ;
	nu->zTopOffset = zTopOffset;
	nu->zBottomOffset = zBottomOffset;

	nu->batch = nullptr;

	return nu;
}

void r2::BatchElem::getLocalBounds(Bounds & b) const {
	b.empty();

	if (!tile) return;

	Matrix44 local;
	local.setScale(scaleX, scaleY, 1.0f);
	local.rotateZ(PASTA_RAD2DEG(rotation));
	local.translate(Pasta::Vector3(x, y, 0.0f));

	Vector2 topLeft(local		* Vector2(tile->dx, tile->dy));
	Vector2 topRight(local		* Vector2(tile->dx + tile->width, tile->dy));
	Vector2 bottomLeft(local	* Vector2(tile->dx, tile->dy + tile->height));
	Vector2 bottomRight(local	* Vector2(tile->dx + tile->width, tile->dy + tile->height));

	b.addPoint(topLeft.x, topLeft.y);
	b.addPoint(topRight.x, topRight.y);
	b.addPoint(bottomLeft.x, bottomLeft.y);
	b.addPoint(bottomRight.x, bottomRight.y);
}

r2::Bounds r2::BatchElem::getBounds(r2::Node* relativeTo) const {
	Bounds b;
	getBounds(b, relativeTo);
	return b;
}

void r2::BatchElem::getBounds(Bounds & b, r2::Node* relativeTo) const {
	if (!relativeTo) {
		getLocalBounds(b);
		return;
	}

	if (!batch) {
		getLocalBounds(b);
		return;
	}

	getLocalBounds(b);

	Matrix44 diff = batch->getRelativeMatrix(relativeTo);
	b.transform(diff);
}

bool r2::BatchElem::shouldRender(rs::GfxContext* _g) {
	if (!batch)
		return false;
	if (!visible || !tile || !getTexture() || ((alpha * color.a) <= 0))
		return false;

	if (_g->supportsEarlyDepth) {
		auto blend = blendmode;
		if (blend == r::TransparencyType::TT_INHERIT)
			blend = batch->blendmode;
		if (blend == r::TransparencyType::TT_INHERIT)
			blend = r::TransparencyType::TT_ALPHA;

		const bool earlyDepthCompatible = (blend == r::TransparencyType::TT_CLIP_ALPHA || blend == r::TransparencyType::TT_OPAQUE) && !batch->forceBasicPass;
		switch (_g->currentPass) {
		case rs::Pass::EarlyDepth:		// fallthrough
		case rs::Pass::DepthEqWrite:	return earlyDepthCompatible;
		case rs::Pass::Basic:			return !earlyDepthCompatible;
		};
	}

	return true; // maybe hide transparent batchelem in picking
}

BeType r2::BatchElem::getType() const{
	return BeType::BET_BATCH_ELEM;
}

void r2::BatchElem::load(const r2::Bitmap* src) {
	name = src->name;
	x = src->x;
	y = src->y;
	z = src->z;
	zTopOffset = src->zTopOffset;
	zBottomOffset = src->zBottomOffset;

	beFlags = src->nodeFlags;
	useSimpleZ = src->useSimpleZ;
	visible = src->visible;
	
	if (ownsTile)
		tile->copy(*src->tile);
	else if (!tile) {
		tile = src->tile->clone();
		ownsTile = true;
	} else
		std::cout << "dunno?" << std::endl;

	blendmode = src->blendmode;
	color = src->color;
	alpha = src->alpha;
	useSimpleZ = src->useSimpleZ;
}

BatchElem* BatchElem::fromPool(rd::TileLib* lib, const char* group, r2::Batch* batch ){
	auto a = rd::Pools::elems.alloc();
	if(lib) 
		lib->getTile(group, 0, 0.0, 0.0, a->tile);
	if (batch) 
		batch->add(a);
	return a;
}
