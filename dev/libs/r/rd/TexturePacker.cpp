#include "stdafx.h"

#include <string>
#include <algorithm>

#include "1-files/FileMgr.h"
#include "1-graphics/Texture.h"
#include "1-texts/TextMgr.h"

#include "3-xml/tinyxml/tinyxml.h"

#include "String.hpp"
#include "TexturePacker.hpp"
#include "TileLib.hpp"

using namespace std;
using namespace rd;
using namespace Pasta;

static const char* detectNumBegin(const char* label) {
	const char * p = label + strlen(label);
	while (p >= label) {
		if( (*(p - 1) < '0') || (*(p - 1) > '9'))
			break;
		p--;
	}

	if (p < label) return nullptr;
	return p;
}

static std::string getHeadingLabel(const char* label) {
	const char * pos = detectNumBegin(label);
	if (pos == nullptr)	return label;
	else				return std::string(label).substr(0, pos - label);
}

std::string makeChecksum(TPSlice & slice) {
	std::stringstream st;
	st << slice.name << ",";
	st << slice.x << ",";
	st << slice.y << ",";
	st << slice.width << ",";
	st << slice.height << ",";
	st << slice.frameX << ",";
	st << slice.frameY << ",";
	st << slice.frameWidth << ",";
	st << slice.frameHeight << ",";
	return st.str();
}

TileLib* TexturePacker::load(const std::string& path, TPConf* conf) {
	return parseXmlAndLoad(path, rd::String::replace(path, ".xml",".png"), conf);
}

TileLib* TexturePacker::parseXmlAndLoad(const std::string & path, const std::string & imgPath, TPConf* _conf) {
	TPConf conf;
	if (_conf)
		conf = *_conf;
	conf.treatFoldersAsPrefixes = false;
	return parseXmlAndLoad(path, imgPath,conf);
}

TileLib* TexturePacker::parseXmlAndLoad(const std::string & _path, const std::string & imgPath, const TPConf & conf) {
	vector<pair<string, string>> pages;
	
	const char* tag = "0.xml";
	const char* imgTag = "0.png";
	int first_tag = 0;

	bool isMultiPage = false;

	if (rd::String::endsWith(_path, "0.xml")) {
		isMultiPage = true;
	}

	if (rd::String::endsWith(_path, "1.xml")) {
		tag = "1.xml";
		imgTag = "1.png";
		isMultiPage = true;
		first_tag = 1;
	}
	
	if (isMultiPage) {
		int curPage = 0;
		for (;;){
			int pageNum = first_tag + curPage;

			string nPath = rd::String::replace(_path, tag, 
				to_string(pageNum) + ".xml");

			string texPath = rd::String::replace(imgPath, imgTag,
				to_string(pageNum) + ".png");

			string cvtNpPath = FileMgr::getSingleton()->convertResourcePath(nPath);
			if (!FileMgr::getSingleton()->exists(cvtNpPath))
				break;
			else 
				pages.push_back(pair(cvtNpPath, texPath));
			
			curPage++;
		}
	}
	else
		pages.push_back(pair(FileMgr::getSingleton()->convertResourcePath(_path), imgPath));

	TileLib* lib = new TileLib();
	lib->name = _path;

	unordered_map<std::string, int> slices;
	unordered_map<std::string, vector<int>> anims;

	for (auto p : pages) {
		string path = p.first;
		string imgPath = p.second;
		TiXmlDocument doc;
		const char* data = (const char*)FileMgr::getSingleton()->load(path);
		if (data == nullptr) { delete lib; return nullptr; }
		doc.Parse(data);

		lib->loadImage(imgPath, conf.filter,conf.keepTextureData);
		
		//scan for smart update tag that may be used to update tile definitions
		for (const TiXmlNode* node = doc.FirstChild(); node; node = node->NextSibling())
		{
			const TiXmlComment* com = dynamic_cast<const TiXmlComment*>(node);;
			if (com) {
				const char* val = com->Value();
				const char* tag = "$TexturePacker:SmartUpdate:";
				const char* tagRest = strstr(val, tag);
				if (tagRest)
					lib->setPageUpdateTag(tagRest+strlen(tag));
			}
		}
		TiXmlElement* root = doc.RootElement();

		if (root->ValueTStr() != "TextureAtlas") 
			continue;

		int width = 0;
		root->QueryIntAttribute("width", &width);
		int height = 0;
		root->Attribute("height", &height);
		TiXmlElement* cur = (TiXmlElement*)root->FirstChild("SubTexture");
		while (cur != nullptr)
		{
			TPSlice sl;
			const char * subName = cur->Attribute("name");
			sl.name = cur->Attribute("name");

			if (sl.name == "elevator/elevatorBigBattery") {
				int po = 0;
			}
			cur->QueryIntAttribute("x", &sl.x);
			cur->QueryIntAttribute("y", &sl.y);
			cur->QueryIntAttribute("width", &sl.width);
			cur->QueryIntAttribute("height", &sl.height);

			int result = 0;
			result = cur->QueryIntAttribute("frameX", &sl.frameX);				if (result) sl.frameX = 0;
			result = cur->QueryIntAttribute("frameY", &sl.frameY);				if (result) sl.frameY = 0;
			result = cur->QueryIntAttribute("frameWidth", &sl.frameWidth);		if (result) sl.frameWidth = sl.width;
			result = cur->QueryIntAttribute("frameHeight", &sl.frameHeight);	if (result) sl.frameHeight = sl.height;

			//get slice name without ext
			const char* ext = strstr(sl.name.c_str(), ".");
			if (ext)
				sl.name = sl.name.substr(0, sl.name.find("."));

			if (strstr(sl.name.c_str(), "/")) {
				if (conf.treatFoldersAsPrefixes) 
					std::replace(sl.name.begin(), sl.name.end(), '/', '_');
				
				else 
					sl.name = sl.name.substr(sl.name.rfind("/")+1);
			}

			//let's remove tailing number
			sl.name = getHeadingLabel(sl.name.c_str());

			int frame = 0;
			rd::String::getLastNumber(subName, frame );
			sl.frame = frame;

			RealFrame rf(sl.frameX, sl.frameY, sl.frameWidth, sl.frameHeight);

			if(!conf.skipSubFrames)
				lib->sliceCustom(subName, 0, sl.x, sl.y, sl.width, sl.height, rf);

			lib->sliceCustom(sl.name, sl.frame, sl.x, sl.y, sl.width, sl.height, rf);

			if( rd::String::endsWith(sl.name,"_")){
				lib->sliceCustom(sl.name.substr(0,sl.name.length()-1), sl.frame, sl.x, sl.y, sl.width, sl.height, rf);
			}

			if (anims.find(sl.name) == anims.end()) {
				vector<int> v;
				v.push_back(sl.frame);
				anims.insert(std::pair<std::string, std::vector<int>>(sl.name, v));
			}
			else
				anims.at(sl.name).push_back(sl.frame);
			cur = (TiXmlElement*)root->IterateChildren("SubTexture", cur);
		}
		FileMgr::getSingleton()->release((void*)data);
	}
	for (auto it = anims.begin(); it != anims.end(); it++) {
		auto& vec = (*it).second;
		sort(vec.begin(), vec.end());
		lib->defineAnim((*it).first, (*it).second);
	}
	return lib;
}