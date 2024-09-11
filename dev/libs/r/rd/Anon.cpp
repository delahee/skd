#include "stdafx.h"
#include "Anon.hpp"
#include "r/Types.hpp"

using namespace std;
using namespace rd;

static const char* z = "";

rd::Anon::Anon(const char * name) {
	mkVoid();
	if (name)	this->name = name;
	else		this->name.clear();
}

rd::Anon::Anon(Anon&&val) {
	data = val.data;
	type = val.type;
	flags = val.flags;

	val.flags &= ~AFL_OWNS_DATA;
	val.mkVoid();
}

void rd::Anon::dispose(){
	if (sibling) 
		sibling = sibling->destroy();
	sibling = 0;

	if (child) 
		child = child->destroy();
	child = 0;

	freeData();
	name.clear();
}

rd::Anon* rd::Anon::destroy(){
	rd::Anon* s = sibling;
	if (!s) 
		s = child;
	else
		s->addChild(child);
	
	sibling = 0;
	child = 0;

	if( flags & AFL_POOLED)
		rd::Pools::anons.fastFree(this);
	else 
		delete this;

	return s;
}

rd::Anon::~Anon(){
	freeData();
	
	if (flags & AFL_POOLED)
		int here = 0;

	if (sibling) {
		sibling = sibling->destroy();
		sibling = 0;
	}
	if (child) {
		child = child->destroy();
		child = 0;
	}
}

bool& rd::Anon::asBool(){
	return reinterpret_cast<bool&>(data);
}

float& rd::Anon::asFloat() {
	return reinterpret_cast<float&>(data);
}

int& rd::Anon::asInt() {
	return reinterpret_cast<int&>(data);
};

void * rd::Anon::asPtr() {
	return data;
};

int * rd::Anon::asIntBuffer() { 
	return (int *)data; 
};

float *	rd::Anon::asFloatBuffer() { 
	return (float *)data; 
};

float* rd::Anon::asVertex3Buffer() {
	return (float*)data;
}

r::Color& rd::Anon::asColor(){
	return *(r::Color*)(data);
}

std::string	rd::Anon::asStdString() {
	string r;
	r.assign((char*)data, this->byteSize-1);
	return r;
}

Str rd::Anon::asStr() {
	if (type != AType::AString)
		return Str(z);
	return Str((char*)data);
};

StrRef rd::Anon::asStrRef() {
	if (type != AType::AString)
		return StrRef(z);
	return StrRef((char*)data);
};

char*rd::Anon::asString() { 
	if (type != AType::AString)
		return 0;
	return (char*)data;
};

int64_t& rd::Anon::asInt64() {
	return reinterpret_cast<int64_t&>(data);
}

uint64_t& rd::Anon::asUInt64(){
	return reinterpret_cast<uint64_t&>(data);
}

double& rd::Anon::asDouble(){
	return reinterpret_cast<double&>(data);
}

void rd::Anon::reserve(int size){
	if (size > byteSize ) {
		data = realloc(data, size);
		byteSize = size;
	}
}

int rd::Anon::getSize(){
	if (type == AType::AIntBuffer || type == AType::AFloatBuffer)
		return byteSize >> 2;
	else
		return byteSize;
}

rd::Anon* rd::Anon::mkVoid() {
	freeData();

	data = nullptr;
	type = rd::AType::AVoid;
	byteSize = 0;
	return this;
}

void rd::Anon::setFloat(float val) {
	if (type != AType::AFloat)
		mkFloat(val);
	else 
		reinterpret_cast<float&>(data) = val;
}

void rd::Anon::setInt(int val) {
	if (type != AType::AInt)
		mkInt(val);
	else
		reinterpret_cast<int&>(data) = val;
}

void rd::Anon::setPtr(void * val){
	if (type != AType::APtr)
		mkPtr(val);
	else
		data = val;
}

void rd::Anon::setVertex3Buffer(const float* v, int nbVertex3) {
	int bSize = nbVertex3 * 3 * 4;
	if (   (type != AType::AFloatBuffer && typeEx!= ATypeEx::AVec3)
		|| (bSize != byteSize))
		mkVertex3Buffer(v, nbVertex3);
	else {
		memcpy(data, v, bSize);
	}
}

void rd::Anon::setFloatBuffer(const float * v, int nbFloats){
	int bSize = nbFloats * 4;
	if (	type != AType::AFloatBuffer
		||	bSize != byteSize)
		mkFloatBuffer(v, nbFloats);
	else {
		memcpy(data, v, bSize);
	}
}

void rd::Anon::aliasFloatBuffer(float * v, int nbFloats) {
	freeData();
	byteSize = nbFloats * sizeof(float);
	data = v;
	type = rd::AType::AFloatBuffer;
	//do not set AFL_OWNS_DATA
}

const rd::Anon* rd::Anon::getByName(const char* _name) const {
	if (name == _name)
		return this;
	if (sibling) {
		rd::Anon* sib = sibling->getByName(_name);
		if (sib)return sib;
	}
	if (child) {
		rd::Anon* c = child->getByName(_name);
		if (c) return c;
	}
	return nullptr;
}

rd::Anon* rd::Anon::getByName(const char* _name) {
	if ( 0 == strcmp(name.c_str() , _name))
		return this;
	if (sibling ) {
		rd::Anon* sib = sibling->getByName(_name);
		if (sib)return sib;
	}
	if (child) {
		rd::Anon* c = child->getByName(_name);
		if (c) return c;
	}
	return nullptr;
}

void rd::Anon::traverse(std::function<void(Anon*)> f){
	f(this);
	if (sibling) sibling->traverse(f);
	if (child) child->traverse(f);
}

void rd::Anon::unitTest()
{
	{
		vector<rd::Anon*> insertList;

		insertList.push_back(rd::Anon::createInt(1, "root"));
		insertList.push_back(rd::Anon::createInt(66, "h0"));
		insertList.push_back(rd::Anon::createInt(67, "h1"));
		insertList.push_back(rd::Anon::createInt(68, "h2"));

		insertList[0]->addChild(insertList[1]);
		insertList[0]->addChild(insertList[2]);
		insertList[0]->addChild(insertList[3]);

		rd::Anon* root = insertList[0];
		assert(root->isSorted());
		assert(root->countHierarchy() == 4);
	}

	{
		vector<rd::Anon*> insertList;

		insertList.push_back(rd::Anon::createInt(1, "root"));
		insertList.push_back(rd::Anon::createInt(66, "h0"));
		insertList.push_back(rd::Anon::createInt(67, "h1"));
		insertList.push_back(rd::Anon::createInt(68, "h2"));

		insertList[0]->addChild(insertList[3]);
		insertList[0]->addChild(insertList[2]);
		insertList[0]->addChild(insertList[1]);

		rd::Anon* root = insertList[0];
		
		assert(root->isSorted());
		assert(root->countHierarchy() == 4);
	}

	{
		vector<rd::Anon*> insertList;

		insertList.push_back(rd::Anon::createInt(1, "root"));
		insertList.push_back(rd::Anon::createInt(66, "h0"));
		insertList.push_back(rd::Anon::createInt(67, "h1"));
		insertList.push_back(rd::Anon::createInt(68, "h2"));

		insertList[0]->addChild(insertList[1]);
		insertList[2]->addChild(insertList[3]);
		insertList[1]->addChild(insertList[2]);

		rd::Anon* root = insertList[0];
		int k = 0;
		assert(root->isSorted());
		assert(root->countHierarchy() == 4);
	}
	//ensure sibs are sorted
	int k = 0;
}

rd::Anon * rd::Anon::createInt(int val,const char * name){
	auto a = rd::Anon::fromPool(name);
	a->mkInt(val);
	return a;
}

string rd::Anon::getValueAsString() {
	switch (type)
	{
	case rd::AType::AVoid:			return "";
	case rd::AType::AFloat:			return to_string(asFloat());
	case rd::AType::AInt:			return to_string(asInt());
	case rd::AType::AString:		return string(asString());
	case rd::AType::AIntBuffer:		return "<todo>";
	case rd::AType::AFloatBuffer:	return "<todo>";
	case rd::AType::AByteBuffer:	return "<todo>";
	case rd::AType::APtr:			return "<todo>";
	case rd::AType::AInt64:			return to_string(asInt64());
	case rd::AType::AUInt64:		return to_string(asUInt64());
	case rd::AType::ADouble:		return to_string(asDouble());
	default:
		return "";
	}
}

string rd::Anon::toString() const {
	return name + " t:" + to_string((int)type) + " tx:" + to_string((int)typeEx);
}

rd::Anon * rd::Anon::createFloat(float val, const char * name ) {
	auto a = rd::Anon::fromPool(name);
	a->mkFloat(val);
	return a;
}



rd::Anon* rd::Anon::clone(bool recursive) const {
	rd::Anon * c = rd::Anon::fromPool(name.c_str());
	
	c->type = type;
	c->typeEx = typeEx;

	if (recursive) {
		c->sibling = sibling ? sibling->clone() : nullptr;
		c->child = child ? child->clone() : nullptr;
	}
	else
		c->sibling = c->child = nullptr;

	switch (c->type) {
		case AType::AInt64:
		case AType::AFloat:
		case AType::AInt:
			c->data = data;
			break;

		default:
			if (flags & AFL_OWNS_DATA) {
				c->data = malloc(byteSize);
				memcpy(c->data, data, byteSize);
			}
			else {
				c->data = data;
			}
			c->byteSize = byteSize;
			break;
	}
	return c;
}

bool rd::Anon::isSorted() {
	bool sorted = true;
	if (sibling) {
		if (!(name < sibling->name))
			return false;
		sorted &= sibling->isSorted();
	}
	if (!sorted) return false;
	if (child) sorted &= child->isSorted();
	return sorted;
}

int rd::Anon::countHierarchy() const{
	int res = 1;
	if (child) res += child->countHierarchy();
	if (sibling) res += sibling->countHierarchy();
	return res;
}

void rd::Anon::addSibling(rd::Anon * sib) {
	if (!sib) return;

	if (sibling == nullptr) {
		sibling = sib;
		return;
	}

	//siling!=nullptr
	if (sibling->name < sib->name) {
		sibling->addSibling(sib);
		return;
	}

	Anon * toReinsert = nullptr;
	if (sib->sibling) toReinsert = sib->sibling;

	Anon * oldSib = sibling;
	sib->sibling = oldSib;
	sibling = sib;

	if(toReinsert) oldSib->addSibling(toReinsert);
}

void rd::Anon::addChild(rd::Anon * c) {
	if (!c)return;
	if (child == nullptr) {
		child = c;
		return;
	}

	if (child->name < c->name) {
		child->addSibling(c);
		return;
	}
	else {
		Anon * toReinsert = nullptr;
		if (c->sibling) toReinsert = c->sibling;

		Anon * oldChild = child;
		c->sibling = oldChild;
		child = c;
		if (toReinsert) oldChild->addSibling(toReinsert);
	}
	
}

rd::Anon* rd::Anon::mkInt(int v) {
	freeData();
	reinterpret_cast<int&>(data) = v;
	type = rd::AType::AInt;
	byteSize = 0;
	return this;
}

rd::Anon* rd::Anon::mkInt64(int64_t v) {
	freeData();
	//if this is a problme on ark 32, fetch the adress we have a padding zone _pad for an authorized overflow
	reinterpret_cast<int64_t&>(data) = v;
	type = rd::AType::AInt64;
	asInt64() = v;
	return this;
}

rd::Anon* rd::Anon::mkDouble(double v) {
	freeData();
	//if this is a problme on ark 32, fetch the adress we have a padding zone _pad for an authorized overflow
	reinterpret_cast<double&>(data) = v;
	type = rd::AType::ADouble;
	return this;
}

bool rd::Anon::setValueFromString(AType t, ATypeEx ate, const char* val) {
	switch (ate) {
	case ATypeEx::ABool:
		mkInt(rd::String::parseBool(val));
		typeEx = ate;
		return true;
		break;
	}
	return setValueFromString(t,val);
}

bool rd::Anon::setValueFromString(AType t, const char* val){
	switch(t){
	case rd::AType::AVoid:			return true;
	case rd::AType::AFloat:			mkFloat(atof(val)); return true;
	case rd::AType::AInt:			mkInt(atoi(val)); return true;
	case rd::AType::AString:		mkString(val); return true;
	case rd::AType::AIntBuffer:		return false;
	case rd::AType::AFloatBuffer:	return false;
	case rd::AType::AByteBuffer:	return false;
	case rd::AType::APtr:			return false;
	case rd::AType::AInt64:			mkInt64(atoll(val)); return true;
	case rd::AType::AUInt64:		mkInt64(atoll(val)); return true;
	case rd::AType::ADouble:		mkDouble(atof(val)); return true;
	default:
		break;
	}
	return false;
}

void rd::Anon::updateString(const std::string& v){
	if (type != rd::AType::AString)
		mkString(v);
	else {
		int strSize = v.size();
		if (byteSize <= strSize)
			byteSize = strSize+1;
		data = realloc(data, byteSize);
		((char*)data)[strSize] = 0;
		memcpy((char*)data, v.data(), strSize);
	}
}

void rd::Anon::updateUInt64(u64 v)
{
	if (type != rd::AType::AUInt64)
		mkUInt64(v);
	else
		asUInt64() = v;
}

rd::Anon* rd::Anon::mkUInt64(uint64_t v){
	freeData();

	asUInt64() = v;

	type = rd::AType::AUInt64;
	return this;
}

rd::Anon* rd::Anon::mkBool(bool b){
	mkInt(b);
	return this;
}

rd::Anon* rd::Anon::mkFloat(float v) {
	freeData();
	reinterpret_cast<float&>(data) = v;		
	type = rd::AType::AFloat;
	byteSize = 0;
	return this;
}

rd::Anon* rd::Anon::mkMatrix44(float * v) {
	mkFloatBuffer(v, 16);
	typeEx = ATypeEx::AMat44;
	return this;
}

rd::Anon* rd::Anon::mkByteBuffer(const uint8_t * bytes, int size) {
	freeData();
	byteSize = size;
	data = malloc(byteSize);
	if (bytes) {
		uint8_t * val = reinterpret_cast<uint8_t *>(data);
		memcpy(val, bytes, byteSize);
	}
	else 
		memset(data, 0, byteSize);
	flags |= AFL_OWNS_DATA;
	type = AType::AByteBuffer;
	return this;
}

rd::Anon* rd::Anon::mkVertex3Buffer(const float* v, int nbVertex) {
	freeData();
	byteSize = nbVertex * sizeof(float) * 3;
	data = malloc(byteSize);
	float* val = reinterpret_cast<float*>(data);
	if (v)	memcpy(val, v, byteSize);
	else	memset(data, 0, byteSize);
	flags |= AFL_OWNS_DATA;
	type = AType::AFloatBuffer;
	typeEx = ATypeEx::AVec3;
	return this;
}

rd::Anon* rd::Anon::mkFloatBuffer(const float * v, int nbFloats) {
	freeData();
	byteSize = nbFloats * sizeof(float);
	data = malloc(byteSize);
	float * val = reinterpret_cast<float *>(data);
	if (v)	memcpy(val, v, byteSize);
	else	memset(data, 0, byteSize);
	flags |= AFL_OWNS_DATA;
	type = AType::AFloatBuffer;
	return this;
}

rd::Anon* rd::Anon::mkIntBuffer(const int * v, int nbInts) {
	freeData();
	byteSize = nbInts * sizeof(int);
	data = malloc(byteSize);
	int * val = (int *)data;
	if (v)	memcpy(val, v, byteSize);
	else	memset(data, 0, byteSize);
	flags |= AFL_OWNS_DATA;
	type = AType::AIntBuffer;
	return this;
}

rd::Anon* rd::Anon::mkVec2(const r::Vector2& v) {
	mkFloatBuffer(v.ptr(), 2);
	typeEx = ATypeEx::AVec2;
	return this;
}

rd::Anon* rd::Anon::mkVec3(const r::Vector3& v) {
	mkFloatBuffer(v.ptr(), 3);
	typeEx = ATypeEx::AVec3;
	return this;
}

rd::Anon* rd::Anon::mkVec4(const r::Vector4& v) {
	mkFloatBuffer(v.ptr(), 4);
	typeEx = ATypeEx::AVec4;
	return this;
}

rd::Anon* rd::Anon::mkColor(const r::Color& v) {
	mkFloatBuffer(v.ptr(), 4);
	typeEx = ATypeEx::AColor;
	return this;
}

rd::Anon* rd::Anon::mkString(const char* str){
	if (!str) return 0;
	int len = strlen(str);
	mkByteBuffer((uint8_t*)str, len+1);
	type = AType::AString;
	return this;
}

rd::Anon* rd::Anon::mkString(const std::string& v){
	return mkString(v.c_str());
}

rd::Anon* rd::Anon::mkPtr(void * ptr){
	freeData();
	data = ptr;
	type = rd::AType::APtr;
	byteSize = 0;
	return this;
}

bool rd::Anon::isBuffer(){
	return (type == AType::AString || type == AType::AIntBuffer || type == AType::AFloatBuffer || type == AType::AByteBuffer ) && data;
}

void rd::Anon::freeData(){
	if (isBuffer()) {
		if (flags & AFL_OWNS_DATA){
			free(data);
			data = nullptr;
			byteSize = 0;
		}
		else {
			data = nullptr;
			byteSize = 0;
		}
		flags &= ~AFL_OWNS_DATA;
	}
	
	data = {};
#ifdef ENVIRONMENT32
	_pad = {};
#endif
	type = rd::AType::AVoid;
	typeEx = rd::ATypeEx::AExVoid;
	byteSize = 0;
}

r2::Node* rd::Anon::asNodePtr() {
	if (typeEx == ATypeEx::ANodePtr)
		return (r2::Node*) asPtr();
	return nullptr;
}

r2::BatchElem* rd::Anon::asBatchElemPtr() {
	if (typeEx == ATypeEx::ABatchElemPtr)
		return (r2::BatchElem*) asPtr();
	return nullptr;
}

rd::Anon* rd::Anon::mkNodePtr(r2::Node* ptr) {
	mkPtr(ptr);
	typeEx = ATypeEx::ANodePtr;
	return this;
}

rd::Anon* rd::Anon::mkAgentPtr(rd::Agent * ptr){
	mkPtr(ptr);
	typeEx = ATypeEx::AAgentPtr;
	return this;
}

rd::Anon* rd::Anon::mkBatchElemPtr(r2::BatchElem* ptr) {
	mkPtr(ptr);
	typeEx = ATypeEx::ABatchElemPtr;
	return this;
}

rd::Anon* rd::Anon::destroyByName(const char* _name) {
	if (name == _name)
		return destroy();
	else {
		if(child)
			child = child->destroyByName(_name);
		if (sibling)
			sibling = sibling->destroyByName(_name);
		return this;
	}
}

int rd::Anon::getMemorySize()
{
	int sz = sizeof(this) + byteSize;
	if (child) sz += child->getMemorySize();
	if (sibling) sz += sibling->getMemorySize();
	return sz;
}

rd::Anon* rd::Anon::fromPool(const char* name){
	auto an = rd::Pools::anons.alloc();
	if (name) an->name = name;
	else
		an->name.clear();
	return an;
}

std::vector<float> rd::Anon::getFloatBufferCopy() {
	float* buf = asFloatBuffer();
	int size = getSize();
	std::vector<float> nbuf;
	nbuf.resize(size,0.0f);
	memcpy(nbuf.data(), buf, size * 4);
	return nbuf;
}


void rd::Anon::operator=(const Anon& rhs)
{
	this->freeData();

	name = rhs.name;
	type = rhs.type;
	typeEx = rhs.typeEx;
	sibling = rhs.sibling;
	child = rhs.child;

	switch (rhs.type) {
	case AType::AFloat:
	case AType::AInt:
		data = rhs.data;
		break;

	default:
		if (rhs.flags & AFL_OWNS_DATA) {
			data = malloc(rhs.byteSize);
			memcpy(data, rhs.data, rhs.byteSize);
		}
		else {
			data = rhs.data;
		}
		byteSize = rhs.byteSize;
		break;
	}

}

bool rd::Anon::operator==(const Anon& rhs) const{
	bool basicEq =
		rd::String::equals(name.c_str(), rhs.name.c_str())
		&& type == rhs.type
		&& typeEx == rhs.typeEx;
	if (!basicEq)
		return false;
	if (rhs.flags & AFL_OWNS_DATA) 
		return 0 == memcmp(&data, &rhs.data, sizeof(data));
	else 
		return 0 == memcmp(&data, &rhs.data, rhs.byteSize);
}
