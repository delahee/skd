#include "stdafx.h"
#include "Texels.hpp"

using namespace r2;
using namespace ext;

r::u8 Texels::getTexelU8(Pasta::TextureData* td, int x, int y){
	int tW = 0;
	r::u8 texel8 = 0;
	r::u8* texel = td->getRawTexel(x, y, tW);
	texel8 = texel[0];
	return texel8;
}

r::u32 Texels::getTexelU32(Pasta::TextureData* td, const r2::Tile* t, int x, int y){
	int tW = 0;
	int lx = t->x + x;
	int ly = t->y + y;
	r::u32 texelU32 = 0;
	r::u8* texel = td->getRawTexel(lx, ly, tW);
	switch (tW) {
	case 1://splat
		texelU32 = texel[0] | texel[0] << 8 | texel[0] << 16;
	case 3:
		memcpy(&texelU32, texel, 3);
		break;
	case 4:
		memcpy(&texelU32, texel, 4);
		break;
	default:
		break;
	}
	return texelU32;
}

void Texels::setTexelU8(Pasta::TextureData* td, int x, int y, r::u8 val) {
#ifdef _DEBUG
	if( (x + y * td->width) >= (td->width * td->height))
		throw "assert : overflow guaranteed";
#endif // DEBUG

	const int twidth = 1;
	u32 toffset = (x + y * td->width) * twidth;
	void * tex = &((u8*)(td->images[0].pixels[0]))[toffset];
	((r::u8*) tex)[0] = val;
}

void Texels::setTexel(Pasta::TextureData* td, int x, int y, r::u8 val){
	int tW = 0;
	r::u8* texel = td->getRawTexel(x, y, tW);
	switch (tW) {
	case 1:
		texel[0] = val;
		break;
	//not sure if splatting is a good idea
	case 3:
		texel[0] = val;
		texel[1] = val;
		texel[2] = val;
		break;
	case 4:
		texel[0] = val;
		texel[1] = val;
		texel[2] = val;
		texel[3] = val;
		break;
	}
}

void Texels::setTexel(Pasta::TextureData* td, int x, int y, r::Color val){
	int tW = 0;
	r::u8* texel = td->getRawTexel(x, y, tW);
	switch( tW ){
	case 1:
		texel[0] = val.r;
		break;
	case 3:
		texel[0] = std::rint(val.r * 255);
		texel[1] = std::rint(val.g * 255);
		texel[2] = std::rint(val.b * 255);
		break;
	case 4:
		texel[0] = std::rint(val.r * 255);
		texel[1] = std::rint(val.g * 255);
		texel[2] = std::rint(val.b * 255);
		texel[3] = std::rint(val.a * 255);
		break;
	}
}

r::Color Texels::getTexel(Pasta::TextureData* td, const r2::Tile* t, int x, int y) {
	int tW = 0;
	int lx = t->x + x;
	int ly = t->y + y;
	r::u8* texel = td->getRawTexel(lx, ly, tW);
	r::Color col = fromTexData(texel, tW);
	return fromTexData(texel, tW);
}

r::Color Texels::random(Pasta::TextureData* td, const r2::Tile* t) {
	auto& rand = rd::Rand::get();
	for (int i = 0; i < 16; ++i) {
		int tW = 0;
		int x = t->x + rand.dice(0, t->width);
		int y = t->y + rand.dice(0, t->height);
		r::u8* texel =
			td->getRawTexel(x,y, tW);
		r::Color col = fromTexData(texel, tW);
		if (col.a > 0)
			return col;
	}
	int tW = 0;
	r::u8* texel =
		td->getRawTexel(t->x + t->width * 0.5f, t->y + t->y + t->height * 0.5f, tW);
	return fromTexData(texel, tW);
}

Vector2 Texels::randomPos(Pasta::TextureData* td, const r2::Tile* t) {
	auto& rand = rd::Rand::get();
	for (int i = 0; i < 16; ++i) {
		int tW = 0;
		int x = t->x + rand.dice(0, t->width);
		int y = t->y + rand.dice(0, t->height);
		r::u8* texel = td->getRawTexel(x,y, tW);
		r::Color col = fromTexData(texel, tW);
		if (col.a > 0) 
			return Vector2(x - t->x,y - t->y);
	}
	return Vector2(0.5f * t->width, 0.5f * t->height);
}

Vector2 Texels::randomPos(Pasta::TextureData* td) {
	auto& rand = rd::Rand::get();
	for (int i = 0; i < 16; ++i) {
		int tW = 0;
		int x = rand.dice(0, td->width);
		int y = rand.dice(0, td->height);
		r::u8* texel = td->getRawTexel(x, y, tW);
		r::Color col = fromTexData(texel, tW);
		if (col.a > 0)
			return Vector2(x, y);
	}
	return Vector2(0.5f * td->width, 0.5f * td->height);
}