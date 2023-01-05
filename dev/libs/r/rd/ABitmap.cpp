#include "stdafx.h"
#include "../r2/Bitmap.hpp"
#include "../r2/Sprite.hpp"

#include "ABitmap.hpp"

#include "1-graphics/geo_vectors.h"
#include "1-graphics/Graphic.h"

using namespace std;
using namespace r2;
using namespace rd;

#define SUPER Bitmap

rd::ABitmap::ABitmap(r2::Node * _parent) : SUPER( _parent ) {
	player.spr = this;
#ifdef _DEBUG
	name = string("Animated#") + to_string(uid);
#endif
}


ABitmap::ABitmap( const char * _groupName, TileLib * _lib, r2::Node * _parent ) : SUPER( _parent ) {
	if (_lib)
		lib = _lib;
	if (_groupName)
		groupName = _groupName;

	player.spr = this;

	set(lib, groupName.c_str(), 0);
	player.playAndLoop(groupName.c_str());
	#ifdef _DEBUG
	name = std::string("Animated#") + std::to_string(uid);
	#endif
}


ABitmap::~ABitmap() {
	dispose();
}

void ABitmap::playAndLoop(const char * g) {
	if (g) groupName = g;
	set(lib, groupName.c_str(), 0);
	player.playAndLoop(g);
}

void rd::ABitmap::replay(int nb)
{
	player.play(groupName.c_str(), nb);
}

void rd::ABitmap::stop()
{
	player.stop();
}

bool ABitmap::isReady() {
	return !destroyed && lib != nullptr && groupName.size();
}

void ABitmap::update(double dt)
{
	float speed = lib ? lib->speed : 0.0;
	player.update(dt * speed );
	SUPER::update(dt);
}

void ABitmap::setWidth(float _w) {
	if (!tile)return;
	scaleX = _w / tile->width;
}

void ABitmap::setHeight(float _h) {
	if (!tile)return;
	scaleY = _h / tile->height;
}

void ABitmap::setFrame(int _frame){
	if (player.onFrame) player.onFrame(frame);

	frame = _frame;

	if (isReady()) {
		frameData = lib->getFrameData(groupName.c_str(), frame);
		syncTile();
	}
}

r::Vector2 rd::ABitmap::getCenterRatio()
{
	if(!usePivot)
		return r::Vector2(0.0,0.0);
	else 
		return r::Vector2(pivotX, pivotY);
}

void rd::ABitmap::setCenterRatio(double px, double py){
	pivotX = px;
	pivotY = py;
	usePivot = true;
	set(lib, groupName.c_str(), frame, false);
}

void rd::ABitmap::setCenterRatioPx(int px, int py) {//I have no idea what i am doing
	if (!frameData)return;
	pivotX = 1.0 * px / frameData->realFrame.realWid;
	pivotY = 1.0 * py / frameData->realFrame.realHei;
	usePivot = true;
	set(lib, groupName.c_str(), frame, false);
}

void ABitmap::setFlippedX(bool onOff) {
	flippedX = onOff;
	syncTile();
}

void ABitmap::setFlippedY(bool onOff) {
	flippedY = onOff;
	syncTile();
}

void ABitmap::set(TileLib * l, const std::string & str, int frame, bool stopAllAnims) {
	set(l, str.c_str(), frame, stopAllAnims);
}

void ABitmap::set(TileLib * l, const char * g, int frame, bool stopAllAnims){
	if (!tile) {
		tile = rd::Pools::tiles.alloc();
		ownsTile = true;
	}

	if (l != nullptr) {
		if (g == nullptr) {
			groupName = "";
			group = nullptr;
		}

		//don't suscribe
		lib = l;
		tile->setTexture( lib->tile->getTexture());
		//don't suscribe

		if( !usePivot ) setCenterRatio(lib->defaultCenterX, lib->defaultCenterY);
		player.frameRate = lib->defaultFrameRate;
	}

	if (g != nullptr && g != groupName)
		groupName = g;

	if (isReady()) {
		if (stopAllAnims) player.stop();

		group = lib->getGroup(groupName.c_str());
		frameData = lib->getFrameData(groupName.c_str(), frame);

		if(frameData==nullptr)
			return;

		setFrame(frame);
	}
}

Node * rd::ABitmap::clone(Node * n)
{
	if (!n) n = new ABitmap();
	
	SUPER::clone(n);

	ABitmap * s = dynamic_cast<ABitmap*>(n);
	if (s) {
		s->destroyed = destroyed;
		s->flippedX = flippedX;
		s->flippedY = flippedY;
		s->usePivot = usePivot;
		s->pivotX = pivotX;
		s->pivotY = pivotY;
		s->frame = frame;
		s->frameData = frameData;
		s->lib = lib;
		s->groupName = groupName;
		s->group = group;
		s->player.copy(player);
		s->player.spr = s;
	}

	return n;
}

void ABitmap::dispose() {
	SUPER::dispose();
	flippedX = false;
	flippedY = false;
}

void rd::ABitmap::play(const char* g, bool loop){
	if (g) groupName = g;
	player.play(g);
	if (loop) player.loop();
}

Tile * ABitmap::syncTile() {
	if (!isReady())
		return tile;

	if (!tile)
		return nullptr;

	FrameData * fd = frameData;
	if (!frameData)
		return tile;

	tile->setTexture(lib->textures[fd->texSlot]);
	tile->setPos(fd->x, fd->y);
	tile->setSize(fd->wid, fd->hei);

	if (usePivot) {
		tile->dx = -(int)(fd->realFrame.x + fd->realFrame.realWid * pivotX);
		tile->dy = -(int)(fd->realFrame.y + fd->realFrame.realHei * pivotY);
	}

	if(flippedX) tile->flipX();
	if(flippedY) tile->flipY();

	return tile;
}

rd::ABitmap* rd::ABitmap::fromPool(TileLib* l, const char* str, r2::Node* parent){
	auto a = rd::Pools::abitmaps.alloc();
	a->set(l);
	a->playAndLoop(str);
	if(parent) parent->addChild(a);
	return a;
}

rd::ABitmap* rd::ABitmap::make(TileLib* l, const char * str, r2::Node* parent) {
	return new rd::ABitmap(str,l, parent);
}

rd::ABitmap* rd::ABitmap::fromLib(TileLib* l, const char* str, r2::Node* parent){
	return make(l,str,parent);
}

void rd::ABitmap::setLib(TileLib* _lib){
	this->lib = _lib;
	player.frameRate = lib->defaultFrameRate;
}

rd::ABitmap* rd::ABitmap::mk(const char* _groupName, TileLib* _lib, r2::Node* _parent){
	auto b = rd::Pools::abitmaps.alloc();
	b->set(_lib);
	b->playAndLoop(_groupName);
	_parent->addChild(b);
	return b;
}

Bounds	rd::ABitmap::getMyLocalBounds() {
	if (!isReady())
		return r2::Bitmap::getMyLocalBounds();
	FrameData* fd = frameData;
	if (!frameData)
		return r2::Bitmap::getMyLocalBounds();
		
	Bounds b;
	b.empty();
	syncAllMatrix();
	if (!tile) return b;

	Pasta::Matrix44 local = getLocalMatrix();
	
	float minx = -fd->realFrame.x - fd->realFrame.realWid * pivotX;
	float miny = -fd->realFrame.y - fd->realFrame.realHei * pivotY;

	float maxx = minx + fd->wid;
	float maxy = miny + fd->hei;

	Pasta::Vector3 topLeft = local * Pasta::Vector3(minx, miny, 0.0);
	Pasta::Vector3 topRight = local * Pasta::Vector3(maxx, miny, 0.0);
	Pasta::Vector3 bottomLeft = local * Pasta::Vector3(minx, maxy, 0.0);
	Pasta::Vector3 bottomRight = local * Pasta::Vector3(maxx, maxy, 0.0);

	b.addPoint(topLeft.x, topLeft.y);
	b.addPoint(topRight.x, topRight.y);
	b.addPoint(bottomLeft.x, bottomLeft.y);
	b.addPoint(bottomRight.x, bottomRight.y);

	return b;
}

#undef SUPER