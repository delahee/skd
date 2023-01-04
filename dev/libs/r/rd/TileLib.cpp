#include "stdafx.h"

#include <algorithm>
#include <vector>
#include <locale>
#include <cctype>

#include "1-graphics/Texture.h"

#include "r2/Lib.hpp"
#include "r2/Rect.hpp"
#include "r2/Tile.hpp"
#include "r2/im/TilePicker.hpp"

#include "TileLib.hpp"

#include "rd/String.hpp"

using namespace rd;
using namespace std;

bool rd::TileLib::RETAIN_TEXTURE_DATA = false;

rd::TileLib::TileLib(){
	r2::im::TilePicker::registerTileSource(this);
}

rd::TileLib::~TileLib(){
	r2::im::TilePicker::unregisterTileSource(this);
	tex = nullptr;
	rd::Pools::tiles.free(tile);
	tile = nullptr;
}

TileLib* rd::TileLib::mock(const TileLib*src, TileLib* allocated) {
	TileLib* nu = 0;
	
	if (!allocated) {
		nu = new TileLib();
		r2::im::TilePicker::unregisterTileSource(nu);
	}
	else
		nu = allocated;

	nu->name = src->name;
	nu->groups = src->groups;
	nu->defaultCenterX = src->defaultCenterX;
	nu->defaultCenterY = src->defaultCenterY;
	nu->defaultFrameRate = src->defaultFrameRate;
	nu->speed = src->speed;
	nu->updateTags = src->updateTags;
	nu->defaultTileName = src->defaultTileName;
	nu->hasChanged = src->hasChanged;

	if (src->tex)	nu->tex = src->tex;
	else			nu->tex = 0;

	if (src->tile)	nu->tile = src->tile->clone();
	else			nu->tile = 0;

	if (src->textures.size())	nu->textures = src->textures;
	else						nu->textures.clear();

	for (auto ts : nu->tiles) {
		ts->clear();
		ts->toPool();
	}
	nu->tiles.clear();
	for (auto ts : src->tiles) {
		auto t = ts->clone();
		nu->tiles.push_back(t);
	}

	for (auto& gs : src->groups) 
		gs.second->lib = nu;
	
	return nu;
}

void rd::TileLib::sliceCustom(const std::string & groupName, int frame, int x, int y, int wid, int hei, RealFrame & rf) {
	TileGroup * g = createGroup(groupName.c_str());
	g->maxWid = max(g->maxWid, wid);
	g->maxHei = max(g->maxHei, hei);

	std::vector<FrameData> & frames = g->frames;

	int delta = frame + 1 - frames.size();
	for (int i = 0; i < delta; i++) {
		FrameData fdr;
		frames.push_back(fdr);
	}

	FrameData & fd = frames.at(frame);
	
	fd.x = x;
	fd.y = y;
	fd.wid = wid;
	fd.hei = hei;
	fd.realFrame = rf;
	fd.rect = r2::Rect(x, y, wid, hei);

	fd.hasPivot = false;
	fd.pX = 0.0;
	fd.pY = 0.0;
	fd.texSlot = textures.size()-1;
}
  
TileGroup * TileLib::getGroup(const char * k) {
	auto val = groups.find(StrRef(k));
	if (val == groups.end())
		return nullptr;
	else 
		return (*val).second;
}

r2::Tile * rd::TileLib::_getTile(const std::string &  str ) {
	return getTile(str,0,0.0,0.0);
}

r2::Tile * rd::TileLib::getTile(const std::string & str, int frame, float px, float py, r2::Tile * res) {
	if (frame < 0) frame = 0;
	return getTile(str.c_str(),frame,px,py, res);
}

r2::Tile * rd::TileLib::getTile(const char * str, int frame, float px, float py, r2::Tile * res){
	if (frame < 0) frame = 0;
	FrameData * fd = getFrameData( str, frame);
	if (fd == nullptr ) {
#ifdef _DEBUG
		cout << "unable to find frame :" << str << endl;
#endif
		if (defaultTileName.size() > 0) 
			return getTile(defaultTileName, 0, px, py, res);
		else 
			return res;
	}
	
	r2::Tile * t = res ? res : (rd::Pools::tiles.alloc());

	t->copy(*tiles[fd->texSlot]);
	t->setPos(fd->x, fd->y);
	t->setSize(fd->wid, fd->hei);

	t->dx = - (int) (fd->realFrame.x + fd->realFrame.realWid * px);
	t->dy = - (int) (fd->realFrame.y + fd->realFrame.realHei * py);

	return t;
}

FrameData * TileLib::getFrameData(const char * str, int frame){
	auto g = getGroup(str);
	if (g == nullptr)	return nullptr;
	else {
		if (g->anim.size()) {
			if (frame >= g->anim.size())
				frame = g->anim.size() - 1;
			return &(g->frames[g->anim[frame]]);
		}
		else {
			if (frame >= g->frames.size())
				frame = g->frames.size() - 1;
			return &(g->frames[frame]);
		}
	}
}


bool rd::TileLib::isAnim(const char * k){
	return exists(k,1);
}

TileGroup * TileLib::createGroup(const char * k) {
	auto ref = StrRef(k);
	auto pos = groups.find(ref);
	if (pos == groups.cend())
		groups[ref] = new TileGroup(k,this);
	return groups[ref];
}

void rd::TileLib::destroy(){
	if(tex) r2::Lib::destroyTexture(tex);
	tex = 0;
}

void rd::TileLib::setDefaultTile(const char * name){
	defaultTileName = name;
}

//todo retain key tiles / i frames ?
bool rd::TileLib::loadImage(const std::string & path, r2::TexFilter filter /*= false*/, bool keepTextureData /*= false*/){
	r::Texture * ltex = r2::Lib::getTexture(path, filter, true , keepTextureData);
	if (!ltex) return false;
	tex = ltex;
	tex->setPath(path.c_str());
	tile = r2::Tile::fromTexture(tex);
	tile->enableTextureOwnership(false);
	textures.push_back(tex);
	tiles.push_back(tile);
	return true;
}

void TileLib::defineAnim(const std::string & name, std::vector<int> & frames) {
	TileGroup * gr = createGroup(name.c_str());
	gr->anim = frames;
}

bool TileLib::exists(const string & k, int frame) {
	return exists(k.c_str(), frame);
}

bool TileLib::exists(const char * _k, int frame ) {
	StrRef k(_k);
	return groups.find(k) != groups.end() && groups[k]->frames.size()>frame;
}

bool rd::TileLib::hasSimilar(const char* grp){
	for (auto & tg  : groups) 
		if (rd::String::stristr( tg.first.c_str(), grp))
			return true;
	return false;
}

void rd::TileLib::getSimilar(const char* grp, rd::TileGroup*& res) {
	for (auto& tg : groups)
		if (rd::String::stristr(tg.first.c_str(), grp)) {
			res = tg.second;
			return;
		}
}

const char* rd::TileLib::getSimilar(const char* grp) {
	for (auto& tg : groups)
		if (rd::String::stristr(tg.first.c_str(), grp))
			return tg.second->id.c_str();
	return nullptr;
}


void TileLib::setPageUpdateTag(const char* updateTag) {
	updateTags.push_back(updateTag);
}

string TileLib::getTagSignature() {
	string basis;
	for (auto& t : updateTags)
		basis += t;
	return basis;
}


void TilePackage::serialize(Pasta::JReflect* j, const char* _name ) {
	j->visitObjectBegin(_name);
	j->visit(lib, "lib");
	j->visit(group, "group");
	if (j->isReadMode()) {
		auto l = r2::im::TilePicker::getOrLoadLib(lib);
		if(l)
			l->getTile(group, 0, 0.0, 0.0, tile);
		else {

		}
	}
	j->visitObjectEnd(_name);
}

TilePackage::TilePackage(const TilePackage& tp) {
	lib = tp.lib;
	group = tp.group;
	if (!tile)
		tile = r2::Tile::fromWhite();
	tile->copy(*tp.tile);
};

TilePackage::TilePackage() {
	if (!tile)
		tile = r2::Tile::fromWhite();
};

TilePackage::~TilePackage() {
	if (tile)
		tile->toPool();
	tile = 0;
};

void TilePackage::im() {
	using namespace ImGui;
	if (TreeNode("Tile Package")) {
		Value("lib", lib);
		Value("group", group);
		if (Button("Change Tile")) {
			Promise* p = r2::im::TilePicker::forPackage(*this);
			p->autoReleased();
			p->then([this](auto self, auto data) {
				dirty = true;
				return data;
			});
		}
		if (TreeNode("Tile")) {
			tile->im();
			TreePop();
		}
		TreePop();
	}
}

rd::TileLib* TilePackage::getLib() {
	if(!cachedLib)
		cachedLib = r2::im::TilePicker::getOrLoadLib(lib);
	return cachedLib;
}

void rd::TileLib::setup(r2::Bitmap* to, const char* grp, int frm, float px, float py){
	if (!to) return;
	if (!grp) return;
	auto frameData = getFrameData(grp, frm);
	FrameData* fd = frameData;
	if (!frameData)
		return;

	auto tile = to->tile;
	tile->setTexture(textures[fd->texSlot]);
	tile->setPos(fd->x, fd->y);
	tile->setSize(fd->wid, fd->hei);

	tile->dx = -(int)(fd->realFrame.x + fd->realFrame.realWid * px);
	tile->dy = -(int)(fd->realFrame.y + fd->realFrame.realHei * py);

	return ;
}

void rd::TileLib::setup(r2::BatchElem* to, const char* grp, int frm, float px, float py){
	if (!to) return;
	if (!grp) return;
	FrameData* fd = getFrameData(grp, frm);
	if (!fd)
		return;
	if (!tile)
		tile = r2::Tile::fromWhite();

	auto tile = to->tile;
	tile->setTexture(textures[fd->texSlot]);
	tile->setPos(fd->x, fd->y);
	tile->setSize(fd->wid, fd->hei);

	tile->dx = -(int)(fd->realFrame.x + fd->realFrame.realWid * px);
	tile->dy = -(int)(fd->realFrame.y + fd->realFrame.realHei * py);
}