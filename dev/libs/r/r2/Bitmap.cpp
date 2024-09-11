#include "stdafx.h"

#include "Bitmap.hpp"

using namespace std;
using namespace r2;

Bitmap::Bitmap(r2::Tile * t, Node * parent ) : Super(parent) {
	if (t == nullptr) {
		tile = rd::Pools::tiles.alloc();
		ownsTile = true;
	}
	else {
		tile = t;
		ownsTile = false;
	}
	setName("Bitmap");
}

r2::Bitmap::Bitmap(Pasta::Texture * t, Node * parent) : Super(parent) {
	tile = Tile::fromPool(t, 0, 0, t->getWidth(), t->getHeight());
	ownsTile = true;
	setName("Bitmap");
}

r2::Bitmap::Bitmap(Node * parent) : Super(parent) {
	tile = rd::Pools::tiles.alloc();
	ownsTile = true;
	setName("Bitmap");
}

Node * r2::Bitmap::clone(Node * n) {
	if (!n) n = rd::Pools::bitmaps.alloc();
	
	Super::clone(n);

	Bitmap * s = dynamic_cast<Bitmap*>(n);
	if (s) {//we may sometime want to transfer only parent!
		s->tile = tile->clone();
		s->ownsTile = true;
	}
	return n;
}

Bitmap::~Bitmap() {
	
}

void r2::Bitmap::releaseTile() {
	if (tile && ownsTile) {
		if(tile->mustDestroyTex())
			tile->disposeTexture();

		if ( tile->isPooled() ) {
			rd::Pools::tiles.release(tile);
			tile = nullptr;
		}
		else {
			delete(tile);
			tile = nullptr;
		}
	}

	//no own no touch get rid of this
	tile = nullptr;
	ownsTile = false;
}

void Bitmap::dispose(){
	Super::dispose();

	releaseTile();
}

void r2::Bitmap::reset() {
	r2::Sprite::reset();

	//avoid touching tiles here please dispose will do it 
}

Bitmap * r2::Bitmap::fromImageFile(const char* path, r2::Node * parent, r2::TexFilter filter ){
	auto tile = Tile::fromImageFile(path,filter);
	auto bmp = new Bitmap(tile,parent);
	bmp->ownsTile = true;
	bmp->name = std::string(path)+"#"+to_string(rs::Sys::getUID());
	bmp->vars.set("r2::bitmap::path", path);
	return bmp;
}

Bitmap* r2::Bitmap::fromBatchElem(r2::BatchElem * be) {
	auto bmp = r2::Bitmap::fromTile(be->tile, be->batch->parent, false);
	bmp->x = be->x;
	bmp->y = be->y;
	bmp->z = be->z;

	bmp->useSimpleZ = be->useSimpleZ;
	bmp->zTopOffset = be->zTopOffset;
	bmp->zBottomOffset = be->zBottomOffset;

	bmp->color = be->color;
	bmp->alpha = be->alpha;
	bmp->visible = be->visible;

	bmp->scaleX = be->scaleX;
	bmp->scaleY = be->scaleY;
	bmp->rotation = be->rotation;
	bmp->blendmode = be->blendmode;

	if (be->batch) {
		bmp->x += be->batch->x;
		bmp->y += be->batch->y;
		bmp->z += be->batch->z;
		bmp->depthRead = be->batch->depthRead;
		bmp->depthWrite = be->batch->depthWrite;
		bmp->killAlpha = be->batch->killAlpha;
		bmp->color *= be->batch->color;
		bmp->alpha *= be->batch->alpha;
		if (bmp->blendmode == r::TransparencyType::TT_INHERIT)
			bmp->blendmode = be->batch->blendmode;
	}

	if (bmp->blendmode == r::TransparencyType::TT_INHERIT)
		bmp->blendmode = r::TransparencyType::TT_ALPHA;

#ifdef _DEBUG
	bmp->name = "be clone#" + to_string(rs::Sys::getUID());
#endif
	return bmp;
}

Bitmap* r2::Bitmap::fromTexture(Pasta::Texture* tex, r2::Node* parent) {
	auto bmp = rd::Pools::bitmaps.alloc();
	bmp->setName("Textured Bitmap");
	bmp->setTile(r2::Tile::fromTexture(tex), true);
	if (parent) parent->addChild(bmp);
	return bmp;
}

Bitmap * r2::Bitmap::fromTile(r2::Tile * t, r2::Node * parent, bool own){
	auto bmp = rd::Pools::bitmaps.alloc();
	bmp->setName("Tiled Bitmap");
	bmp->setTile(t, own);
	if(parent) parent->addChild(bmp);
	return bmp;
}


Bitmap* r2::Bitmap::fromColor24(r::u32 col, r2::Node* parent) {
	return fromColor(r::Color::fromUInt24(col), parent);
}

Bitmap* r2::Bitmap::fromColor32(r::u32 col, r2::Node* parent) {
	return fromColor(r::Color::fromUInt(col), parent);
}

Bitmap* r2::Bitmap::fromColor(const r::Color& c, r2::Node* parent) {
	auto bmp = fromTile(r2::GpuObjects::whiteTile->clone(), parent, true);
	bmp->setName("Colored Bitmap");
	bmp->color = c;
	bmp->vars.set("rd::TileColor", c);
	return bmp;
}

Tile * r2::Bitmap::getPrimaryTile() {
	return tile;
}

void Bitmap::setWidth(float _w){
	if (!tile) return;
	scaleX = _w / tile->width;
}

void Bitmap::setHeight(float _h){
	if (!tile) return;
	scaleY = _h / tile->height;
}

Bounds r2::Bitmap::getMyLocalBounds(){
	Bounds b;
	b.empty();
	syncAllMatrix();

	if (!tile) return b;

	Pasta::Matrix44 local = getLocalMatrix();
	
	Pasta::Vector3 topLeft		= local * Pasta::Vector3(tile->dx,					tile->dy, 0.0);
	Pasta::Vector3 topRight		= local * Pasta::Vector3(tile->dx + tile->width,	tile->dy, 0.0);
	Pasta::Vector3 bottomLeft	= local * Pasta::Vector3(tile->dx,					tile->dy + tile->height, 0.0);
	Pasta::Vector3 bottomRight	= local * Pasta::Vector3(tile->dx + tile->width,	tile->dy + tile->height, 0.0);

	b.addPoint(topLeft.x, topLeft.y);
	b.addPoint(topRight.x, topRight.y);
	b.addPoint(bottomLeft.x, bottomLeft.y);
	b.addPoint(bottomRight.x, bottomRight.y);

	return b;
}

r::Vector2 r2::Bitmap::getCenterRatio() {
	return Vector2( tile->getCenterRatioX(), tile->getCenterRatioY());
}

void r2::Bitmap::setCenterRatio(double px, double py){
	if (tile) {
		tile->setCenterRatio(px, py);
		vars.set("c_px", px);
		vars.set("c_py", py);
	}
}

void r2::Bitmap::setCenterRatioPixel(int x, int y){
	setCenterRatio(1.0f * x / tile->width, 1.0f * y / tile->height);
}

void r2::Bitmap::loadTileByPtr(const r2::Tile * t){
	if( tile ) tile->copy(*t);
	else {
		tile = t->clone();
		ownsTile = true;
	}
}

void r2::Bitmap::setTile(r2::Tile * t, bool own) {
	releaseTile();
	tile = t;
	ownsTile = own;
}

void r2::Bitmap::copyTile(const r2::Tile * t) {
	if (tile) tile->copy(*t);
}

void r2::Bitmap::loadTileByRef(const r2::Tile & t){
	if (tile) tile->copy(t);
}

std::string r2::Bitmap::toString()
{
	return std::string("bmp ") + (!tile ? "" : tile->toString());
}

Bitmap* r2::Bitmap::fromLib(rd::TileLib* t, const char* name, r2::Node* parent) {
	if (t == nullptr) {
#ifdef _DEBUG
		throw "no such library, test library existence first";
#endif
		return nullptr;
	}
	return fromTile(t->getTile(name), parent, true);
}

Bitmap* r2::Bitmap::fromLib(rd::TileLib* t, const std::string& name, r2::Node* parent) {
	return fromLib(t, name.c_str(), parent);
}

