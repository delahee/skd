#include "stdafx.h"

#include <functional>

#include "1-json/jsoncpp/json.h"
#include "1-files/Path.h"
#include "r2/Node.hpp"
#include "r2/Sprite.hpp"
#include "r2/Bitmap.hpp"
#include "rd/ABitmap.hpp"
#include "rd/TileLib.hpp"
#include "rd/TexturePacker.hpp"

#include "HedJsonLevel.hpp"
//#include "1-json/Json.h"

//#include "rapidjson/Pasta/rapidjson.h"

using namespace r2;
using namespace Pasta;
using namespace fmt;
using namespace fmt::hed;
using namespace Json;

//rapidjson::Value v;

fmt::HedJsonLevel::HedJsonLevel(HedJsonLevelConf conf){
	this->conf = conf;
}

void fmt::HedJsonLevel::parseAndLoad(std::string path, r2::Node * scene){
	json = rd::RscLib::getJSON(path);
	loadToScene(json.root, scene);
}

bool fmt::HedJsonLevel::loadToScene( Json::Value * root, r2::Node * scene) {
	loadGfx(root);

	Value vscene = root->get("scene", Value(ValueType::objectValue));
	loadRec( vscene, scene);
	resetTRS(scene);
	cman.load(vscene,scene);
	return true;
}

fmt::hed::TileOrigin fmt::HedJsonLevel::getOrigin(Value & tile) {
	Value origin = tile.get("origin",ValueType::objectValue);
	fmt::hed::TileOrigin to;

	to.bmp		= origin.get("bmp", Value("")).asString();
	to.sheet	= origin.get("sheet", Value("")).asString();
	to.group	= origin.get("group", Value("")).asString();
	to.color	= origin.get("color", Value(0)).asInt();
	to.pivotX	= origin.get("pivotX", Value(0.f)).asDouble();
	to.pivotY	= origin.get("pivotY", Value(0.f)).asDouble();

	return to;
}

void fmt::HedJsonLevel::loadRec( Json::Value & d, r2::Node * current ) {
	r2::Node * node = current;
	node->name = d.get("name", Value("")).asString();

	Value defArray(ValueType::arrayValue);
	Value defObj(ValueType::objectValue);
	Value defFloat(0.0f);
	Value defInt(0);

	Value vPos		= d.get("pos", defArray);
	float x			= vPos[0u].asDouble();
	float y			= vPos[1u].asDouble();

	if (fitPixels)	node->setPos((int)x, (int)y);
	else			node->setPos(x, y);

	Value vScale	= d.get("scale", defArray);
	node->scaleX		= vScale[0u].asDouble();
	node->scaleY		= vScale[1u].asDouble();

	node->rotation	= d.get("rotation", defFloat).asDouble();
	node->visible	= d.get("visible", Value(true)).asBool();

	r2::Sprite * spr = dynamic_cast<r2::Sprite*>(node);
	r2::Bitmap * bmp = dynamic_cast<r2::Bitmap*>(node);
	rd::ABitmap * abmp = dynamic_cast<rd::ABitmap*>(node);

	if ( spr ) {
		if (d.isMember("color")) {
			Value color = d.get("color", defArray);
			spr->color.r = color[0u].asDouble();
			spr->color.g = color[1u].asDouble();
			spr->color.b = color[2u].asDouble();
			spr->color.a = color[3u].asDouble();
		}

		if (d.isMember("blendMode")) {
			int blend = d.get("blendMode", defInt).asInt();
			switch (blend) {
				case 0: spr->blendmode = TransparencyType::TT_ALPHA; break;	
				case 1:	spr->blendmode = TransparencyType::TT_ADD; break;
				case 2:	spr->blendmode = TransparencyType::TT_MULTIPLY; break;
				case 3: spr->blendmode = TransparencyType::TT_OPAQUE; break;
				//case 4 SoftAdd
				//case 5 Erase
				case 5: spr->blendmode = TransparencyType::TT_ERASE; break;
				//case 6 SoftOverlay
				case 7: spr->blendmode = TransparencyType::TT_SCREEN; break;
				
			}
		}
	}

	Value tile = d.get("tile", defObj);

	if (d.isMember("tile") && abmp) {//here is an animated
		fmt::hed::TileOrigin orig = getOrigin(tile);

		if (orig.sheet.size() > 0) {
			std::string sh = mkShortHand(orig.sheet);
			SlbEntry &blib = slib[sh];
			if (blib.slb->isAnim(orig.group.c_str())) {
				abmp->set(blib.slb);
				abmp->playAndLoop(orig.group.c_str());
			}
			else {
				abmp->set(blib.slb, orig.group.c_str(),0,true);
			}
		}
		else if (orig.bmp.size() > 0) {
			bmp->tile = r2::Tile::fromImageFile(orig.bmp, conf.filter);
		}
		else {
			printf("No proper origin ? \n");
		}
		bmp->setCenterRatio(orig.pivotX, orig.pivotY);
	}
	else if(d.isMember("tile") && !abmp && bmp) {//not animated
		fmt::hed::TileOrigin orig = getOrigin(tile);
		if (orig.bmp.size() > 0) {
			bmp->tile = r2::Tile::fromImageFile(orig.bmp, conf.filter);
		}
		else {
			printf("No proper origin ? \n");
		}
		bmp->setCenterRatio(orig.pivotX, orig.pivotY);
	}
	
	for( Value & child : d.get("children", Value(ValueType::arrayValue))) {
		int i = 0;
		r2::Node * spr = nullptr;

		std::string type = child.get("type", Value("")).asString();

		if(			type == std::string("h2d.Sprite") ){
			spr = new r2::Node(current);
		}				
		else if (	type == std::string("CurveNode") ) {
			spr = new CurveNode(current);
		}				
		else if (	type == std::string("CurveControlPoint")) {
			spr = new CurveControlPoint(current);
		}				
		else if (	type == std::string("h2d.Bitmap")) {
			spr = new r2::Bitmap(current);
		}				
		else if (	type == std::string("mt.deepnight.slb.HSprite")) {
			spr = new rd::ABitmap(current);
		}
		else {
			printf("hmm cannot detect type, bailing out \n");
		}

		if (spr)
			loadRec(child, spr);
	}
	
	
	
}

void fmt::HedJsonLevel::dispose() {
	slib.clear();
	bmap.clear();
}

void fmt::HedJsonLevel::cleanup() {
	slib.clear();
	bmap.clear();
}

bool fmt::HedJsonLevel::loadGfx(Json::Value * d) {
	Value bunch = d->get("libTool", Value(ValueType::objectValue));//{ bmap:Array<String>, slibs:Array<String> }

	Value bmap = bunch.get("bmap", Value(ValueType::arrayValue));

	for (auto bmpPath : bmap) 
		importBitmap(bmpPath.asString());

	Value slibs = bunch.get("slibs", Value(ValueType::arrayValue));
	for (auto slibPath : slibs)
		importLib(slibPath.asString());
	return true;
}

std::string HedJsonLevel::mkShortHand(std::string path) {
	std::string delimiter = "/";
	std::string origPath = path;
	path.erase(0, origPath.rfind(delimiter) + delimiter.length());
	return path;
}


void HedJsonLevel::importBitmap(std::string path) {
	fmt::hed::BmpEntry e;
	e.path = path;
	e.shorthand = mkShortHand(path);
	if (bmap.find(e.shorthand) != bmap.end())
		return;//already found;
	e.tile = r2::Tile::fromImageFile(path);
	bmap[e.shorthand] = e;
}

void HedJsonLevel::importLib(std::string path) {
	int bl = 0;
	fmt::hed::SlbEntry e;
	e.path = path;
	e.shorthand = mkShortHand(path);

	if (slib.find(e.shorthand) != slib.end())
		return;//already found;

	std::string dir = Pasta::Path(path).getDirectory();

	rd::TPConf conf;
	conf.filter = r2::TexFilter::TF_LINEAR;
	Pasta::Path imgPath = Pasta::Path(path);
	imgPath.setExtension("png");
	rd::TileLib * tl = rd::TexturePacker::parseXmlAndLoad(path, imgPath.getStr(), conf);
	e.slb = tl;
	slib[e.shorthand] = e;
}

void HedJsonLevel::resetTRS(r2::Node * sc ) {
	sc->setPos(0, 0);
	sc->setScale(1,1);
	sc->rotation = 0.f;
}

void fmt::CurveMan::load(Json::Value & d, r2::Node * scene)
{
	nodes.clear();
	std::vector<CurveControlPoint*> controls;
	std::vector<CurveNode*> & lnodes = nodes;

	std::function<void(r2::Node * n)> f = [&lnodes, &controls](r2::Node * n) {
		CurveNode* cn = dynamic_cast<CurveNode*>(n);
		if (cn) lnodes.push_back(cn);
		CurveControlPoint* ccp = dynamic_cast<CurveControlPoint*>(n);
		if (ccp) controls.push_back(ccp);
	};
	scene->traverseLastChildFirst(f);

	for (auto n : nodes)
		nodes;

	Json::Value defltArr(ValueType::arrayValue);
	Json::Value defltInt(0);

	Json::Value dataCurves = d.get("curves", defltArr);
	if (dataCurves.isArray()) {

		for (auto n : dataCurves) {
			int start	= n.get("start", defltInt).asInt();
			int end		= n.get("end", defltInt).asInt();
			Value line = n.get("line", defltArr);//array of int

			CurveEntry * entry = new CurveEntry();

			entry->start = nodes[start];
			entry->end = nodes[end];

			std::vector<CurveControlPoint*> cps;
			for (Value vInt : line) {
				int index = vInt.asInt();
				cps.push_back(controls[index]);
			}
			entry->line = CurveLine::fromCPS(cps);
			curves.push_back(entry);
		}
			
	}
}
