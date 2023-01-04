#include "stdafx.h"

#include "ChangePalette.hpp"

#include "1-graphics/Texture.h"
#include "rd/BitArray.hpp"

using namespace r2;
using namespace r2::svc;

static inline uint32_t ctransform(uint32_t src, const Matrix44& mat) {
	r::Vector4 colorVec = r::Color::fromUInt(src).toVec4();
	r::Vector4 trans = mat * colorVec;
	return r::Color::toUInt(trans);
}

r2::svc::ChangePalette::ChangePalette(const std::string & root) {
	path = root;
	memset(&workData, 0, sizeof(workData));
	loadTextureData();
	setMainPalette(extractPalette());
	beginChanges();
}

void r2::svc::ChangePalette::reset(const std::string& root) {
	dispose();
	mainPalette.clear();
	curPalette.clear();
	ranges.clear();
	path = root;
	memset(&workData, 0, sizeof(workData));
	loadTextureData();
	setMainPalette(extractPalette());
	beginChanges();
}

r2::svc::ChangePalette::ChangePalette() {
	memset(&workData, 0, sizeof(workData));
}

r2::svc::ChangePalette::ChangePalette(Pasta::TextureData* data) {
	memset(&workData, 0, sizeof(workData));
	this->data = data;
	setMainPalette(extractPalette());
	beginChanges();
}

r2::svc::ChangePalette::~ChangePalette()
{
	dispose();
	Pasta::TextureLoader * loader = Pasta::TextureLoader::getSingleton();
	loader->release(data);
}

bool r2::svc::ChangePalette::loadTextureData()
{
	Pasta::GraphicContext * ctx = Pasta::GraphicContext::GetCurrent();
	Pasta::TextureLoader * loader = Pasta::TextureLoader::getSingleton();
	data = loader->load(path);
	return data != nullptr;
}

eastl::vector<uint32_t> r2::svc::ChangePalette::extractPalette(){
	eastl::unordered_map<uint32_t, uint32_t> bits;

	for (int i = 0; i < Pasta::TextureData::MAX_MIPMAP_LEVELS; i++) {
		int size = data->images[0].sizes[i];
		if (size == 0) continue;

		int nbCols = (!size) ? 0 : size >> 2;
		void* rawData = data->images[0].pixels[i];
		uint32_t * colors = (uint32_t *)rawData;
		for (int k = 0; k < nbCols; k++) {
			uint32_t color = colors[k];
			bits[color] = color;
		}
	}

	eastl::vector<uint32_t> res;
	for (auto kv : bits)
		res.push_back(kv.first);
	return res;
}

eastl::vector<uint32_t> r2::svc::ChangePalette::extractPalette(Pasta::TextureData * data) {
	eastl::unordered_map<uint32_t, uint32_t> pal;
	uint32_t pos = 0;
	for (Pasta::TextureData::Image & img : data->images) {
		for (int i = 0; i < Pasta::TextureData::MAX_MIPMAP_LEVELS; i++) {
			if (img.sizes[i] == 0) continue;
			int nbCol = img.sizes[i] >> 2;
			uint32_t * ptr = (uint32_t*)img.pixels[i];
			for (int k = 0; k < nbCol; ++k) {
				uint32_t color = ptr[k];
				if (pal.find(color) == pal.end())
					pal[color] = pos++;
			}
		}
	}
	eastl::vector<uint32_t> palResult;
	palResult.resize(pos);
	for (auto& iter : pal) 
		palResult[iter.second] = iter.first;
	return palResult;
}

void r2::svc::ChangePalette::setMainPalette(Pasta::TextureData * data){
	setMainPalette(extractPalette(data));
}

void r2::svc::ChangePalette::setMainPalette(const eastl::vector<uint32_t>& mainPalette){
	if(mainPalette.size())
		this->mainPalette.assign(mainPalette.begin(), mainPalette.end());

	colorToIdx.clear();
	for (int i = 0; i < mainPalette.size(); ++i) 
		colorToIdx[mainPalette[i]] = i;

	curPalette = mainPalette;
}


void r2::svc::ChangePalette::extractRange(const std::string & rangeName, eastl::vector<uint32_t>& paletteToDiff){
	PaletteRange rg;
	rg.name = rangeName;
	int msize = std::min<int>(paletteToDiff.size(), mainPalette.size());
	for (int i = 0; i < msize;++i)
		if (paletteToDiff[i] != mainPalette[i])
			rg.index.push_back(i);
	ranges.push_back(rg);
}

void r2::svc::ChangePalette::resetImage(){
	int imgIdx = 0;
	for (Pasta::TextureData::Image& img : workData.images) {
		auto & remoteImg = data->images[imgIdx];
		for (int i = 0; i < 16; ++i)
			if (img.pixels[i]) 
				memcpy(img.pixels[i], remoteImg.pixels[i], remoteImg.sizes[i]);
		imgIdx++;
	}
}

void r2::svc::ChangePalette::resetRange(const std::string& rangeName)
{
	int imgIdx = 0;
	for (Pasta::TextureData::Image& img : workData.images) {
		for (int mipIdx = 0; mipIdx < 16; ++mipIdx) {
			u32* pixLine = (u32*)data->images[imgIdx].pixels[mipIdx];
			for (u32 i = 0; i < data->images[imgIdx].sizes[mipIdx]>>2; i++) {
				u32 pix = pixLine[i];
				if (isInRange(rangeName.c_str(), pix))
					((u32*)img.pixels[mipIdx])[i] = pix;
			}
		}
		imgIdx++;
	}
}

void r2::svc::ChangePalette::addRange(const std::string & rangeName, eastl::vector<uint32_t>& index){
	if (index.size() <= 0)return;
	PaletteRange rg;
	rg.name = rangeName;
	if(index.size())
		rg.index.assign( &index[0], &index[0] + index.size());
	ranges.push_back(rg);
}

void r2::svc::ChangePalette::dispose() {
	if (workData.images.size()) {
		for (Pasta::TextureData::Image & img : workData.images) {
			for( int i = 0 ;i < 16;++i)
				if (img.pixels[i]) {
					free(img.pixels[i]);
					img.pixels[i] = nullptr;
				}
		}
		workData.images.clear();
	}
}

void r2::svc::ChangePalette::beginChanges(){
	if (!data) 
		return;

	dispose();
	workData = *data;

	Pasta::TextureData::Image & nuImage = workData.images[0];

	for (int i = 0; i < Pasta::TextureData::MAX_MIPMAP_LEVELS; i++) {
		int sz = data->images[0].sizes[i];
		nuImage.sizes[i] = sz;
		if (sz) {
			nuImage.pixels[i] = malloc(sz);
			memcpy(nuImage.pixels[i], data->images[0].pixels[i], sz);
		}
		else 
			nuImage.pixels[i] = nullptr;
	}
	curPalette = mainPalette;
}

void r2::svc::ChangePalette::apply(const eastl::unordered_map<uint32_t, uint32_t> & changeMap) {
	for (int i = 0; i < Pasta::TextureData::MAX_MIPMAP_LEVELS; i++) {
		int size = data->images[0].sizes[i];
		if (size == 0) continue;
		int nbCols = (!size) ? 0 : size >> 2;
		void* rawData = data->images[0].pixels[i];
		uint32_t * colors = (uint32_t *)rawData;
		uint32_t * dstColors = (uint32_t *)(workData.images[0].pixels[i]);
		for (int k = 0; k < nbCols; k++) {
			auto tgtCol = changeMap.find(colors[k]);
			if (tgtCol != changeMap.end()) {
				dstColors[k] = tgtCol->second;
			}
		}
	}
}

void r2::svc::ChangePalette::changeAll(const Pasta::Matrix44 & mat){
	for (int i = 0; i < Pasta::TextureData::MAX_MIPMAP_LEVELS; i++) {
		int size = data->images[0].sizes[i];
		if (size == 0) continue;

		int nbCols = (!size) ? 0 : size >> 2;
		void* rawData = data->images[0].pixels[i];
		uint32_t * colors = (uint32_t *)rawData;
		uint32_t * dstColors = (uint32_t *)(workData.images[0].pixels[i]);
		for (int k = 0; k < nbCols; k++) 
			dstColors[k] = ctransform(colors[k], mat);
	}

	for (int i = 0; i < curPalette.size(); ++i) 
		curPalette[i] = ctransform(mainPalette[i], mat);
}

void r2::svc::ChangePalette::changeAll(const eastl::vector<uint32_t>& nuColors){
	eastl::unordered_map<uint32_t, uint32_t> changeMap;
	int changed = 0;
	for (int i = 0; i < nuColors.size(); ++i) 
		if (curPalette[i] != nuColors[i])
			changeMap[mainPalette[i]] = nuColors[i];

	apply(changeMap);
	
	curPalette = nuColors;
}

void r2::svc::ChangePalette::change(const std::string & name, const Pasta::Matrix44 & mat)
{
	PaletteRange * _rg = getRange(name.c_str());

	if (!PASTA_VERIFY_MSG(_rg, "need a valid change range here")) //need this
		return;

	eastl::unordered_map<uint32_t, uint32_t> changeMap;

	for (int i = 0; i < _rg->index.size(); ++i) {
		uint32_t col = mainPalette[_rg->index[i]];
		uint32_t tcol = ctransform(col, mat);
		changeMap[col] = tcol;
		curPalette[_rg->index[i]] = tcol;
	}
	
	apply(changeMap);
}

//pass a full palette
void r2::svc::ChangePalette::change(const std::string & name, const eastl::vector<uint32_t>& nuPalette){
	PaletteRange * _rg = getRange(name.c_str());

	if (!PASTA_VERIFY_MSG(_rg,"need a valid change range here")) //need this
		return;

	PaletteRange & rg = *_rg;

	eastl::unordered_map<uint32_t, uint32_t> changeMap;
	for(uint32_t & idx : rg.index)
		if (mainPalette[idx] != nuPalette[idx]) {
			changeMap[mainPalette[idx]] = nuPalette[idx];
			curPalette[idx] = nuPalette[idx];
		}

	apply(changeMap);
}

void r2::svc::ChangePalette::changeSlice(const eastl::vector<uint32_t>& _changeIdx, const Pasta::Matrix44 & mat){
	eastl::unordered_map<uint32_t, uint32_t> changeMap;
	for (const uint32_t & idx : _changeIdx ) {
		uint32_t col = ctransform(curPalette[idx], mat);
		changeMap[mainPalette[idx]] = col;
		curPalette[idx] = col;
	}

	apply(changeMap);
}

void r2::svc::ChangePalette::reset(){
	beginChanges();
}

bool r2::svc::ChangePalette::isInRange(const char * rgName,uint32_t color){
	PaletteRange* rg = getRange(rgName);
	if (!rg) return false;
	for (int idx : rg->index)
		if (mainPalette[idx] == color)
			return true;
	return false;
}

PaletteRange * r2::svc::ChangePalette::getRange(const char * rangeName){
	for (PaletteRange&rg : ranges)
		if ( 0 == strncmp(rg.name.c_str(), rangeName, rg.name.size()))
			return &rg;
	return nullptr;
}

