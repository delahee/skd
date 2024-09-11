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

static char cvt[2048] = {0};

bool rd::RscLib::has(const char* path) {
	FileMgr::getSingleton()->convertResourcePath(path,cvt, 2048-4);
	return FileMgr::getSingleton()->exists(cvt);
}

bool rd::RscLib::has(const std::string& path){
	std::string npath = FileMgr::getSingleton()->convertResourcePath(path);
	return FileMgr::getSingleton()->exists(npath);
}

bool rd::RscLib::hasText(const char* path) {
#ifdef _DEBUG
	if (rd::String::startsWith(path, "res")) {
		std::cout << "res should nt be present in the path at this point ARE YOU SURE?" << std::endl;
		return false;
	}
#endif
	std::string npath = FileMgr::getSingleton()->convertResourcePath(path);
	return FileMgr::getSingleton()->exists(npath);
}

char * rd::RscLib::getText(const char* path) {
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

void * rd::RscLib::getBinary(const char *path, r::s64 * sz) {
	std::string npath = FileMgr::getSingleton()->convertResourcePath(path);
	r::u32 fSize = 0;
	void *data = FileMgr::getSingleton()->load(npath, &fSize);
	if(sz) *sz = fSize;
	return data;
}

void rd::RscLib::releaseBinary(void * data){
	FileMgr::getSingleton()->release(data);
}

JSONResource rd::RscLib::getJSON(const std::string &path){	
	if (!has(path)) {
		traceError("no such file "s + path);
		return {};
	}
	JSONResource rsc;
	rsc.data = getBinary(path.c_str());
	
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

void rd::RscLib::compressLZ(const std::string& _src, rd::Buffer& dst) {
	rd::Buffer src;
	src.ptr = (char*) _src.data();
	src.size = _src.size();
	compressLZ(src, dst);
}

void rd::RscLib::compressLZ(const rd::Buffer& src, rd::Buffer& dst){
#ifdef _DEBUG
	if (dst.size != 0) 
		throw std::invalid_argument("dst must be an empty and clear buffer");
	if (src.size < 16)
		throw std::invalid_argument("src must be at least 16b");
#endif
	dst.reset();
	int bufsize = src.size * 1.1;//spec specifies we must have at least 5% more space
	dst.ptr = malloc(bufsize);
	if (!dst.ptr) 
		throw std::bad_alloc{};
	
	memset(dst.ptr, 0, bufsize);
	dst.size = fastlz_compress( src.base(), src.size, dst.base());
	int here = 0;
}

r::u32 rd::RscLib::getDecompressedSizeLZ(const char* src) {
	return ((u32*)src)[0];
}

void rd::RscLib::decompressLZ(const char* src, u32 size, char* dst, u32& decompSize) {
	decompSize = fastlz_decompress(src, size, dst, decompSize);
}


PLZResource rd::RscLib::getPLZ(std::string path) {
	PLZResource rsc;
	r::s64 compressedSize = 0;
	r::u32 * data = (r::u32*) getBinary(path.c_str(), &compressedSize);
	if (data == nullptr)
		return rsc;

	rsc.data = data;
	rsc.uncompressedSize = ((r::u32*)data)[0];
	data++;
	rsc.uncompressedData = malloc(rsc.uncompressedSize);
	memset(rsc.uncompressedData, 0, rsc.uncompressedSize);
	fastlz_decompress(data, (int) compressedSize, rsc.uncompressedData, rsc.uncompressedSize);
	return rsc;
}

void rd::RscLib::releasePLZ(PLZResource & rsc) {
	releaseBinary(rsc.data);
	rsc.data = nullptr;
	free(rsc.uncompressedData);
	rsc.uncompressedData = nullptr;
	rsc.uncompressedSize = 0;
}


Pasta::TextureData* rd::RscLib::getTextureData(const char* path){
	if (rd::String::startsWith(path, "res/"))
		path += strlen("res/");
	return Pasta::TextureLoader::getSingleton()->load(path);
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

bool rd::MemCache::has(const char* path){
	return rs::Std::exists(data,path);
}

Buffer* rd::MemCache::get(const char* path) {
	return rs::Std::get(data, path);
}

void rd::MemCache::set(const char* path, const Buffer& d){
	data[path] = d;
}

void MemCache::im() {
	using namespace ImGui;
	Checkbox("enabled", &enabled);
	for (auto& kv : data) {
		if (TreeNode(kv.first)) {
			Buffer b = kv.second;
			ImGui::Text("hit %d", b.nbHit);
			ImGui::Text("size %ld", b.size);
			TreePop();
		}
	}
}

char* rd::MemCache::getCachedText(const char* path) {
	bool useCache = enabled;
	if (useCache) {
		if (has(path)) {
			Buffer* data = get(path);
			data->nbHit++;
			return (char*)data->ptr+data->ofs;
		}
	}
	r::s64 size = 0;
	void* content = rd::RscLib::getBinary(path, &size);
	if (useCache)
		set(path, { content, size, 0 });
	return (char*)content;
}

void rd::MemCache::prefetchFile(const char* path) {
	if (has(path)) {
		Buffer* data = get(path);
		data->nbHit++;
		return;
	}
	r::s64 size = 0;
	void* content = rd::RscLib::getBinary(path, &size);
	set(path, { content, size, 0 });
}

bool rd::MemCache::getCachedFile(const char* path, Str& dest) {
	bool useCache = enabled;
	if (useCache) {
		if (has(path)) {
			Buffer * data = get(path);
			data->nbHit++;
			dest = StrRef( (char*)data->ptr + data->ofs );
			return true;
		}
	}
	r::s64 size = 0;
	void * content = rd::RscLib::getBinary(path, &size);
	dest = StrRef((char*)content);
	if (useCache)
		set(path, { content, size, 0 });
	return content != 0;
}

MemCache rd::RscLib::fileCache;

void rd::Buffer::reset() {
	ptr = 0;
	size = 0;
	ofs = 0;
	nbHit = 0;
}

void rd::Buffer::dealloc() {
	if (ptr)
		free(ptr);
	ptr = 0;
	size = 0;
	ofs = 0;
	nbHit = 0;
}

void* rd::Buffer::base() const{
	u8* baseU8 =(u8*)ptr + ofs;
	return (void*) baseU8;
}
