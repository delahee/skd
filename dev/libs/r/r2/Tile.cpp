#include "Tile.hpp"
#include "Tile.hpp"
#include "stdafx.h"
#include "1-graphics/Texture.h"
#include "1-graphics/Graphic.h"
#include "1-graphics/GraphicContext.h"
#include "Tile.hpp"
#include <string>
#include "im/TilePicker.hpp"

using namespace std;
using namespace r2;

static int s_uid = 0;

r2::Tile::Tile(){
	uid = s_uid++;
}

Tile::Tile( Pasta::Texture * _tex, int _x, int _y, int _w, int _h, int _dx, int _dy ) : Tile(){
	x = _x;
	y = _y;

	if (_w == -1) _w = _tex->getWidth();
	if (_h == -1) _h = _tex->getHeight();

	width = _w;
	height = _h;

	dx = _dx;
	dy = _dy;
	if( _tex ) setTexture( _tex );
}

r2::Tile::Tile(const Tile & t) : Tile(){
	tex = t.tex;

	x = t.x;//rectangle x upon texture
	y = t.y;//rectangle y upon texture
	width = t.width;//rectangle width upon texture
	height = t.height;//rectangle height upon texture

	dx = t.dx;//dx is the offset to apply for pivots
	dy = t.dy;//dy is the offset to apply for pivots

	flags = 0; //don't inherit tex releasing

	u1 = t.u1;
	v1 = t.v1;

	u2 = t.u2;
	v2 = t.v2;
}

Tile::~Tile() {
	if (flags & R2_TILE_MUST_DESTROY_TEX)
		disposeTexture();
	flags = 0;
}

void Tile::enableTextureOwnership(bool onOff) {
	if(onOff)	flags |= R2_TILE_MUST_DESTROY_TEX;
	else		flags &= ~R2_TILE_MUST_DESTROY_TEX;
}

Tile * r2::Tile::fromPool(Pasta::Texture * tex, int rectX, int rectY, int w, int h, int dx, int dy){
	Tile * t = rd::Pools::tiles.alloc();
	t->set(tex, rectX, rectY, w, h, dx, dy);
	return t;
}

Tile * r2::Tile::fromWhite(){
	return fromPool( GpuObjects::whiteTex );
}

void r2::Tile::toPool(){
	rd::Pools::tiles.free(this);
}

void r2::Tile::mapTexture(Pasta::Texture * _tex)
{
	setCenterDiscrete(0, 0);
	setPos(0, 0);
	setSize(_tex->getLogicalWidth(), _tex->getLogicalHeight());
	setTexture(_tex);
}

r2::Bounds r2::Tile::getBounds(){
	return r2::Bounds::fromTLWH(-dx,-dy, width, height);
}

void r2::Tile::destroy(){
	if(isPooled()) {
		toPool();
	}
	else {
		delete this;
	}
}

void r2::Tile::set(Pasta::Texture * _tex, int _x, int _y, int _w, int _h, int _dx, int _dy){
	x = _x;
	y = _y;

	if (_w == -1 && _tex) _w = _tex->getWidth();
	if (_h == -1 && _tex) _h = _tex->getHeight();

	width = _w;
	height = _h;

	dx = _dx;
	dy = _dy;
	if (_tex) setTexture(_tex);
}

void r2::Tile::clear(){
	if (mustDestroyTex()) 
		disposeTexture();
	tex = nullptr;

	x = 0;//rectangle x upon texture
	y = 0;//rectangle y upon texture
	width = 0;//rectangle width upon texture
	height = 0;//rectangle height upon texture

	dx = 0;//dx is the offset to apply for pivots
	dy = 0;//dy is the offset to apply for pivots

	flags = 0;

	u1 = 0.0;
	v1 = 0.0;

	u2 = 1.0;
	v2 = 1.0;
}

void Tile::disposeTexture(){
	if (tex && !mustDestroyTex() ) {
		rs::trace(string() + "Are you sure you want to destroy "+ tex->getDebugName()+" from its tile?");
	}
	Pasta::GraphicContext * ctx = Pasta::GraphicContext::GetCurrent();
	if(tex)ctx->DestroyTexture(tex);
	tex = nullptr;
	enableTextureOwnership(false);
}

void r2::Tile::setTexture(Pasta::Texture * _tex, double u1, double v1, double u2, double v2){
	tex = _tex;
	if (tex) {
		this->u1 = u1;
		this->v1 = v1;

		this->u2 = u2;
		this->v2 = v2;

		x = u1 * tex->getWidth();
		y = v1 * tex->getHeight();
		
		width = (u2 - u1) * tex->getWidth();
		height = (v2 - v1) * tex->getHeight();
	}

	if (tex->getFlags() & PASTA_TEXTURE_RENDER_TARGET) 
		textureFlipY();
}

void r2::Tile::setCenterDiscrete(double px, double py){
	dx = px;
	dy = py;
}

void Tile::setCenterRatio(double px, double py) {
	dx = - px * width;
	dy = - py * height;
}

void r2::Tile::setUV(double u1, double v1, double u2, double v2)
{
	this->u1 = u1;
	this->u2 = u2;
	this->v1 = v1;
	this->v2 = v2;
}				

Tile * Tile::fromImageFile(const std::string & path, r2::TexFilter filter ){
	Pasta::Texture* tex = r2::Lib::getTexture(path, filter, false);
	if (!tex)
		return nullptr;
	tex->setPath(path.c_str());

	if (!tex) return nullptr;
	Tile* t = fromPool(tex, 0, 0, tex->getWidth(), tex->getHeight());
	t->enableTextureOwnership(true);
	return t;
}

Tile* r2::Tile::fromColor(const r::Color & c) {
	auto ctx = Pasta::GraphicContext::GetCurrent();
	auto loader = Pasta::TextureLoader::getSingleton();
	int texData[16] = {};
	for (int i = 0; i < 16; ++i)
		texData[i] = c.toInt();
	Pasta::Texture* tex= ctx->CreateTexture(loader->loadRaw(texData, 4, 4));
	auto t = new r2::Tile(tex);
	t->setPos(2, 2);
	t->setSize(1, 1);
	return t;
}

Tile * Tile::fromTexture(Pasta::Texture*tex){
	if (!tex) return nullptr;
	Tile * t = fromPool(tex, 0, 0, tex->getWidth(), tex->getHeight());
	return t;
}

Tile * r2::Tile::fromTextureData(Pasta::TextureData * texData, r2::TexFilter filter){
	Pasta::GraphicContext * ctx = Pasta::GraphicContext::GetCurrent();
	Pasta::Texture * tex = ctx->CreateTexture(texData, toPastaTextureFilter( filter ));
	Tile * t = fromPool(tex, 0, 0, tex->getWidth(), tex->getHeight());
	t->enableTextureOwnership(true);
	return t;
}

void r2::Tile::copy(const Tile & t){
	tex = t.tex;

	x = t.x;//rectangle x upon texture
	y = t.y;//rectangle y upon texture
	width = t.width;//rectangle width upon texture
	height = t.height;//rectangle height upon texture

	dx = t.dx;//dx is the offset to apply for pivots
	dy = t.dy;//dy is the offset to apply for pivots

	//flags = ???; //don't say anything about flags

	u1 = t.u1;
	v1 = t.v1;

	u2 = t.u2;
	v2 = t.v2;

	flags |= (t.flags & R2_TILE_TARGET_FLIPED);
}

r2::Tile* r2::Tile::subRectTLDR(const Vector2i & tl, const Vector2i& dr) {
	Tile* t = rd::Pools::tiles.alloc();
	t->copy(*this);
	t->x += tl.x;
	t->y += tl.y;

	t->width = (dr - tl).x;
	t->height = (dr - tl).y;
	return t;
}

r2::Tile* r2::Tile::subRectPosSize(const Vector2i& tl, const Vector2i& size) {
	Tile* t = rd::Pools::tiles.alloc();
	t->copy(*this);
	t->x += tl.x;
	t->y += tl.y;

	t->width = size.x;
	t->height = size.y;
	return t;
}

void r2::Tile::swap(Tile & t){
	Tile temp;
	temp.flags = 0;

	temp.copy(*this);
	copy(t);
	t.copy(temp);
}

Tile * Tile::clone() const {
	Tile * t = rd::Pools::tiles.alloc();
	t->copy(*this);
	return t;
}

bool r2::Tile::isTargetFlip(){
	return flags & R2_TILE_TARGET_FLIPED;
}

void Tile::translateCenterDiscrete(double _dx, double _dy) {
	this->dx += _dx;
	this->dy += _dy;
}

void Tile::translatePos(double _dx, double _dy) {
	this->x += _dx;
	this->y += _dy;
	if (tex) {
		bool wasFlipped = isTargetFlip();
		u1 = x / tex->getWidth();
		v1 = y / tex->getHeight();
		u2 = (width + x) / tex->getWidth();
		v2 = (height + y) / tex->getHeight();
		if (wasFlipped) {
			resetTargetFlip();
			textureFlipY();
		}
	}
}

/*
void Tile::setPos(int _x, int _y) {
	this->x = _x;
	this->y = _y;
	if (tex) {
		u1 = (float)x / tex->getWidth();
		v1 = (float)y / tex->getHeight();
		u2 = (float)(width + x) / tex->getWidth();
		v2 = (float)(height + y) / tex->getHeight();
	}
}

void Tile::setSize(int w, int h) {
	width = w;
	height = h;
	if (tex) {
		u2 = (float)(w + x) / tex->getWidth();
		v2 = (float)(h + y) / tex->getHeight();
	}
}
*/

void r2::Tile::setPos(double _x, double _y)
{
	this->x = _x;
	this->y = _y;
	if (tex) {
		resetTargetFlip();
		u1 = x / tex->getWidth();
		v1 = y / tex->getHeight();
		u2 = (width + x) / tex->getWidth();
		v2 = (height + y) / tex->getHeight();
	}
}

void r2::Tile::setSize(double w, double h)
{
	width = w;
	height = h;
	if (tex) {
		resetTargetFlip();
		u2 = (w + x) / tex->getWidth();
		v2 = (h + y) / tex->getHeight();
	}
}

void r2::Tile::flipX(){
	std::swap(u1, u2);
	dx = -dx - width;
}

void r2::Tile::flipY(){
	std::swap(v1, v2);
	dy = -dy - height;
}

void r2::Tile::resetTargetFlip()
{
	flags &= ~R2_TILE_TARGET_FLIPED;
}

void r2::Tile::targetFlipY() {
	if (!(flags & R2_TILE_TARGET_FLIPED)) {
		std::swap(v1, v2);
		flags |= R2_TILE_TARGET_FLIPED;
	}
}

void r2::Tile::textureFlipY(){
	if (!(flags & R2_TILE_TARGET_FLIPED)) {
		v1 = 1.0 - v1;
		v2 = 1.0 - v2;
		flags |= R2_TILE_TARGET_FLIPED;
	}
}

void r2::Tile::snapToPixel() {
	dx = rint(dx);
	dy = rint(dy);
}

void r2::Tile::setTexture(Pasta::Texture * _tex) {
	tex = _tex;
	if (tex) {
		resetTargetFlip();
		this->u1 = x / tex->getWidth();
		this->v1 = y / tex->getHeight();
		this->u2 = (x + width) / tex->getWidth();
		this->v2 = (y + height) / tex->getHeight();
	}
	if (_tex && (_tex->getFlags() & PASTA_TEXTURE_RENDER_TARGET))
		textureFlipY();
}

u64 r2::Tile::getHash() {
	return (u64) tex 
		+ uid
		+ (u64)(u1 * 1024 * 1024)
		+ (u64)(u2 * 1024 * 1024)
		+ (u64)(v1 * 1024 * 1024)
		+ (u64)(v2 * 1024 * 1024)
		+ (u64)(dx * 1024 * 1024)
		+ (u64)(dy * 1024 * 1024)
	;
}