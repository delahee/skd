#include "stdafx.h"

#include "1-files/FileMgr.h"
#include "1-json/jsoncpp/json.h"
#include "1-json/jsoncpp/reader.h"

#include "RscLib.hpp"
#include "fastlz.h"
#include "BitArray.hpp"

using namespace std;
using namespace Pasta;
using namespace rd;
using namespace Json;

bool									rd::RscLib::allowBitArrayProduction = false;
std::vector<TransientTexture>			rd::RscLib::transientTex;

bool rd::RscLib::hasText(const std::string& path) {
#ifdef _DEBUG
	if (rd::String::startsWith(path, "res")) {
		std::cout << "res should nt be present in the path at this point ARE YOU SURE?" << std::endl;
		return false;
	}
#endif
	std::string npath = FileMgr::getSingleton()->convertResourcePath(path);
	return FileMgr::getSingleton()->exists(npath);
}

char * rd::RscLib::getText(const std::string &path) {
#ifdef _DEBUG
	if (rd::String::startsWith(path,"res")) {
		std::cout << "res should nt be present in the path at this point ARE YOU SURE?" << std::endl;
	}
#endif
	std::string npath = FileMgr::getSingleton()->convertResourcePath(path);
	char *data = (char *)FileMgr::getSingleton()->load(npath);
	return data;
}

void rd::RscLib::releaseText(char * data){
	FileMgr::getSingleton()->release((void*)data);
}

void * rd::RscLib::getBinary(const std::string & path, unsigned int * sz) {
	std::string npath = FileMgr::getSingleton()->convertResourcePath(path);
	void *data = FileMgr::getSingleton()->load(npath,sz);
	return data;
}

void rd::RscLib::releaseBinary(void * data){
	FileMgr::getSingleton()->release(data);
}

JSONResource rd::RscLib::getJSON(const std::string &path){	
	JSONResource rsc;
	rsc.data = getBinary(path);
	
	Json::Reader rd(Json::Features::all());

	Json::Value * root = new Json::Value(ValueType::objectValue);
	rd.parse(std::string((char*)rsc.data), *root);

	rsc.root = root;
	return rsc;
}

void rd::RscLib::releaseJSON(JSONResource & rsc){
	releaseBinary(rsc.data);
	rsc.data = nullptr;
	delete rsc.root; rsc.root = nullptr;
}

void rd::RscLib::compressLZ(const char * src, u32 size, char* dst, u32 & compSize ) {
	compSize = fastlz_compress(src, size, dst);
}

r::u32 rd::RscLib::getDecompressedSizeLZ(const char* src) {
	return ((u32*)src)[0];
}

void rd::RscLib::decompressLZ(const char* src, u32 size, char* dst, u32& decompSize) {
	decompSize = fastlz_decompress(src, size, dst, decompSize);
}


PLZResource rd::RscLib::getPLZ(std::string path) {
	PLZResource rsc;
	Pasta::u32 compressedSize = 0;
	Pasta::u32 * data = (Pasta::u32*)getBinary(path, &compressedSize);
	if (data == nullptr)
		return rsc;

	rsc.data = data;
	rsc.uncompressedSize = ((Pasta::u32*)data)[0];
	data++;
	rsc.uncompressedData = malloc(rsc.uncompressedSize);
	memset(rsc.uncompressedData, 0, rsc.uncompressedSize);
	fastlz_decompress(data, compressedSize, rsc.uncompressedData, rsc.uncompressedSize);
	return rsc;
}

void rd::RscLib::releasePLZ(PLZResource & rsc) {
	releaseBinary(rsc.data);
	rsc.data = nullptr;
	free(rsc.uncompressedData);
	rsc.uncompressedData = nullptr;
	rsc.uncompressedSize = 0;
}


/*file will stay in memory, release it asap*/
Pasta::TextureData*	rd::RscLib::getTextureData(const std::string & path) {
	return Pasta::TextureLoader::getSingleton()->load(path);
}

void rd::RscLib::releaseTextureData(Pasta::TextureData * data) {
	Pasta::TextureLoader::getSingleton()->release(data);
}

static std::unordered_map<Str, rd::BitArray*> bitArrays;

void rd::RscLib::makeBitArray(const char* path, Pasta::TextureData* data){
	if (!allowBitArrayProduction)
		return;

	int size = data->images[0].sizes[0];
	if (size == 0) return;
	int nbColumns = data->getWidth();
	int nbRows = data->getHeight();
	int texelWidth = 0;
	void* rawData = data->images[0].pixels[0];
	auto ptr = data->getRawTexel(0, 0, texelWidth, 0, 0);
	if (texelWidth != 4)
		return;

	rd::BitArray* _ba = new rd::BitArray();
	rd::BitArray& ba = *_ba;
	ba.resize(nbColumns * nbRows);

	for (int x = 0; x < nbColumns; ++x) {
		for (int y = 0; y < nbRows; ++y) {
			int idx = x + y * nbColumns;
			auto ptr = data->getRawTexel(x, y, texelWidth, 0, 0);
			if( texelWidth == 4)
				ba.toggle(idx, ptr[3] != 0);
		}
	}

	bitArrays[Str(path)] = _ba;
	std::string pSimplified = path;

	pSimplified = rd::String::replace(pSimplified, ".png", "");
	bitArrays[Str((pSimplified))] = _ba;

	pSimplified = path;
	pSimplified = rd::String::replace(pSimplified, ".png", ".xml");
	bitArrays[Str(pSimplified)] = _ba;
}

rd::BitArray* rd::RscLib::getBitArray(const char* path){
	auto strp = StrRef(path);
	bool exists = bitArrays.find(strp) != bitArrays.end();
	if (!exists) return nullptr;
	return bitArrays[StrRef(strp)];
}
