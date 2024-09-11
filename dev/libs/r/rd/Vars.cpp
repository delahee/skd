#include "stdafx.h"
#include "Vars.hpp"
#include "rd/JSerialize.hpp"

using namespace std;
using namespace r;
using namespace rd;

static const char* const emptyString  = "";

static rd::Vars s_vEmpty;
static rd::Anon s_aEmpty;

#define POOL_ANONS 1
Vars::Vars() {

}

Vars::Vars(const Vars& ov) {
	if( ov.head)
		head = ov.head->clone();
}

Vars& Vars::operator+=(const Vars& other) {
	if (!other.head) return *this;
	head->addSibling(other.head->clone());
	return *this;
}

Vars& Vars::operator+=(const rd::Anon* e) {
	if (!e)return *this;
	if (has(e->name.c_str()))
		remove(e->name.c_str());
	if(e) 
		add(e->clone());
	return *this;
}

Vars& rd::Vars::operator=(const rd::Anon& e){
	dispose();
	head = e.clone();
	return *this;
}

Vars& rd::Vars::operator=(const rd::Vars& e){
	dispose();
	if(e.head)
		head = e.head->clone();
	return *this;
}

void Vars::traverse(std::function<void(rd::Anon*)> f) {
	if (!head) return;
	head->traverse(f);
}
void Vars::traverse(std::function<void(const rd::Anon*)> f) const{
	if (!head) return;
	head->traverse(f);
}

void Vars::merge(Vars& other) {
	vector<const char*> lsNames; 
	other.traverse([&lsNames](rd::Anon* r) {
		lsNames.push_back(r->name.c_str());
	});

	for (const char* name : lsNames)
		destroy(name);

	this->operator+=(other);
}

void Vars::loadWithFilter(Vars& other, std::vector<std::string>& varFilters) {
	std::vector<const char*> lsNames;
	other.traverse([&lsNames, varFilters](rd::Anon* r) {
		for( auto &str : varFilters)
			if(r->name==str)
				lsNames.push_back(r->name.c_str());
	});
	for (const char* name : lsNames)
		destroy(name);
	this->operator+=(other);
}

rd::Vars::~Vars(){
	while(head) 
		head = head->destroy();
	head = 0;
}

bool Vars::has(const char* name) const{
	if (!head) return false;
	return nullptr != get(name);
}

void Vars::destroy(const char* name) {
	if (!head) return;
	head = head->destroyByName(name);
}

rd::Anon* rd::Vars::safeGet(const char* name) {
	if (!name) return 0;
	auto n = get(name);
	if (!n) n = &s_aEmpty;
	return n;
}

rd::Anon* rd::Vars::safeGet(const char* name) const {
	if (!name) return 0;
	auto n = get(name);
	if (!n) n = &s_aEmpty;
	return n;
}

rd::Anon* Vars::get(const char* name) {
	if (!name) return nullptr;
	return (!head) ? nullptr : head->getByName(name);
}

rd::Anon* Vars::get(const char* name) const{
	if(!name) return nullptr;
	return (!head) ? nullptr : head->getByName(name);
}

rd::Anon* Vars::getOrCreate(const char* _name) {
	if (!_name) return nullptr;

	if (!head) {
#ifdef POOL_ANONS
		head = rd::Anon::fromPool(_name);
#else
		head = new rd::Anon(_name);
#endif
		return head;
	}

	rd::Anon* an = head->getByName(_name);
	if (an) return an;
#ifdef POOL_ANONS
	an = rd::Anon::fromPool(_name);
#else
	an = new rd::Anon(_name);
#endif
	head->addSibling(an);
	return an;
}

rd::Anon* Vars::add(rd::Anon* nmeta) {
	if (!head)
		head = nmeta;
	else
		head->addSibling(nmeta);
	return head;
}

rd::Anon* Vars::set(const char* name, const r::u8* content, int nbBytes) {
	if (!name) return nullptr;
	rd::Anon* meta = getOrCreate(name);
	meta->mkByteBuffer(content, nbBytes);
	return meta;
}

rd::Anon* Vars::set(const char* name, const float* content, int nbFloats) {
	if (!name) return nullptr;
	rd::Anon* meta = getOrCreate(name);
	meta->mkFloatBuffer(content, nbFloats);
	return meta;
}

rd::Anon* Vars::set(const char* name, const int* content, int nbInts) {
	if (!name) return nullptr;
	rd::Anon* meta = getOrCreate(name);
	meta->mkIntBuffer(content, nbInts);
	return meta;
}

rd::Anon * rd::Vars::set(const char * name, const Vector3* content, int nbVec3) {
	if (!name) return nullptr;
	rd::Anon* meta = getOrCreate(name);
	meta->mkByteBuffer(nullptr, nbVec3 * sizeof(Vector3));
	meta->typeEx = ATypeEx::AVec3Array;
	for (int i = 0; i < nbVec3; i++)
		((Vector3*)meta->data)[i] = content[i];
	return meta;
}

rd::Anon * rd::Vars::set(const char * name, const void** content, int nbPtr) {
	if (!name) return nullptr;
	rd::Anon* meta = getOrCreate(name);
	meta->mkByteBuffer(nullptr, nbPtr * sizeof(void*));
	meta->typeEx = ATypeEx::APtrArray;
	for (int i = 0; i < nbPtr; i++)
		((void**)meta->data)[i] = (void*)content[i];
	return meta;
}

rd::Anon* Vars::set(const char* name, const char* content) {
	if (!name) return nullptr;
	rd::Anon* meta = getOrCreate(name);
	if(meta) meta->mkString(content);
	return meta;
}

//todo support colors and vectors
void rd::Vars::setUntyped(const char* name, const char* content) {
	if (rd::String::equalsI(content, "true"))
		set(name, true);
	else if (rd::String::equalsI(content, "false"))
		set(name, false);
	else if (rd::String::isFloat(content))
		set(name, atof(content));
	else if (rd::String::isInteger(content))
		set(name, atoi(content));
	else
		set(name, content);
}

rd::Anon* Vars::set(const char* name, const std::string& content) {
	rd::Anon* meta = getOrCreate(name);
	meta->mkString(content);
	return meta;
}

rd::Anon* Vars::set(const char* name, float content) {
	rd::Anon* meta = getOrCreate(name);
	if (meta) meta->mkFloat(content);
	return meta;
}

rd::Anon* Vars::set(const char* name, double content) {
	rd::Anon* meta = getOrCreate(name);
	if (meta) meta->mkDouble(content);
	return meta;
}

rd::Anon* Vars::set(const char* name, int content) {
	if (!name) return 0;
	rd::Anon* meta = getOrCreate(name);
	if (meta) meta->mkInt(content);
	return meta;
}

rd::Anon* Vars::set(const char* name, bool content) {
	rd::Anon* meta = getOrCreate(name);
	if (meta) 
		meta->mkBool(content);
	return meta;
}

rd::Anon* rd::Vars::set(const char* name, u64 content) {
	rd::Anon* meta = getOrCreate(name);
	meta->mkUInt64(content);
	return meta;
}



rd::Anon* rd::Vars::set(const char* name, r::u32 content) {
	rd::Anon* meta = getOrCreate(name);
	meta->mkInt((int)content);
	return meta;
}

rd::Anon* rd::Vars::set(const char* name, const Vector3& content) {
	rd::Anon* meta = getOrCreate(name);
	meta->mkVertex3Buffer(content.ptr(), 1);
	return meta;
}

rd::Anon* rd::Vars::set(const char* name, const Vector2& content) {
	rd::Anon* meta = getOrCreate(name);
	meta->mkVertex3Buffer(content.ptr(), 1);
	meta->typeEx = ATypeEx::AVec2;
	return meta;
}

rd::Anon* rd::Vars::set(const char* name, const Vector2i& content) {
	rd::Anon* meta = getOrCreate(name);
	meta->mkIntBuffer(content.ptr(), 2);
	meta->typeEx = ATypeEx::AVec2;
	return meta;
}

rd::Anon* Vars::set(const char* name, rd::Agent* ptr) {
	rd::Anon* meta = getOrCreate(name);
	meta->mkAgentPtr(ptr);
	return meta;
}

rd::Anon* Vars::setPtr(const char* name, void* ptr) {
	rd::Anon* meta = getOrCreate(name);
	meta->mkPtr(ptr);
	return meta;
}

rd::Anon* Vars::set(const char* name, r2::Node* ptr) {
	rd::Anon* meta = getOrCreate(name);
	meta->mkNodePtr(ptr);
	return meta;
}

rd::Anon* Vars::set(const char* name, void* content) {
	rd::Anon* meta = getOrCreate(name);
	meta->mkPtr(content);
	return meta;
}

rd::Anon* rd::Vars::set(const char* name, const r::Color& content) {
	auto an = set(name,content.ptr(), 4);
	an->typeEx = rd::ATypeEx::AColor;
	return an;
}

rd::Anon* rd::Vars::set(const char* name, const r::Vector4i& content) {
	auto an = set(name, content.ptr(), 4);
	an->typeEx = rd::ATypeEx::AVec4;
	return an;
}

rd::Anon* rd::Vars::set(const char* name, const r::Vector4& content){
	auto an = set(name, content.ptr(), 4);
	an->typeEx = rd::ATypeEx::AVec4;
	return an;
}

void Vars::incr(const char* name) {
	if (!has(name))
		set(name, 1);
	else {
		int oldVal = getInt(name,0);
		set(name, oldVal + 1);
	}
}

rd::Anon* rd::Vars::searchWithPrefix(const char* prefix) {
	if (!head)
		return nullptr;

	if (rd::String::containsI(head->name.c_str(), prefix))
		return head;
	rd::Anon * cur = head->child;
	while (cur) {
		if (rd::String::containsI(cur->name.c_str(), prefix))
			return cur;
		cur = cur->child;
	}
	cur = head->sibling;
	while (cur) {
		if (rd::String::containsI(cur->name.c_str(), prefix))
			return cur;
		cur = cur->sibling;
	}

	return nullptr;
}

void rd::Vars::cleanByPrefix(const char* prefix) {
	if (!head) return;
	std::function<void(rd::Anon*)>  f = [prefix](rd::Anon* a) {
		if (strstr(a->name.c_str(), prefix))
			a->mkVoid();
	};
	head->traverse(f);

	rd::Anon* cur = searchWithPrefix(prefix);
	while (cur) {
		destroy(cur->name.c_str());
		cur = searchWithPrefix(prefix);
	}
}


bool rd::Vars::im(bool showAsHeader, const char *_name) {
	using namespace ImGui;

	bool changed = false;
	PushID(this);

	std::string name = "Metadata";
	if (_name)
		name = _name;
	name = std::string(ICON_MD_STICKY_NOTE_2 " ") + name;
	bool open = showAsHeader ? 
		ImGui::CollapsingHeader(name.c_str()) :
		ImGui::TreeNode(name.c_str());
	
	if (open) {
		if (showAsHeader) Indent();
		if (head)
			changed |= r2::Im::metadata(head);
		else {
			Text("No data on record.");
			if (Button("Create")) {
				getOrCreate("");
				changed = true;
			}
		}
		if (showAsHeader) Unindent();
		if (!showAsHeader) TreePop();
	}
	PopID();
	return changed;
}

void rd::Vars::serialize(Pasta::JReflect * _jr, const char* name){
	Pasta::JReflect& jr = *_jr;
	if (jr.isReadMode()) { // If we deserialize we need to remove what's already present
		while(head) 
			head = head->destroy();
		head = 0;
	}

	bool newMethod = true;
	if (name && jr.isReadMode()) {
		auto baseObject = Pasta::JsonGet(jr.m_jvalue, name);
		newMethod = baseObject && baseObject->IsObject(); // format detection
	}

	if(name && newMethod)
		jr.visitObjectBegin(name);

	bool hasData = head != 0;
	jr.visit(hasData, "hasData");
	if (hasData && jr.isReadMode())
		head = getOrCreate("");
	if(hasData && head)
		jr.visit(*head, "head");

	if(name && newMethod)
		jr.visitObjectEnd(name);
}

bool Vars::copy(const char* name, Vars& v) {
	rd::Anon* val = v.get(name);
	if (!val) return false;

	if (has(name))
		destroy(name);
	
	add(val->clone(false));
	return true;
}

void rd::Vars::load(const Vars& v){
	dispose();
	if (v.head)
		head = v.head->clone();
}

void Vars::dispose() {
	while(head) 
		head = head->destroy();
	head = 0;
}

bool Vars::getBool(const char* name, bool dflt) const {
	if (has(name)) {
		auto an = get(name);
		if (an->type == AType::AString) {
			char* rep = an->asString();
			if (rd::String::equalsI(rep, "true"))
				return true;
			if (rd::String::equalsI(rep, "false"))
				return false;
		}
		if (an->type != AType::AInt) return dflt;

		return an->asBool();
	}
	return dflt;
}

r::uid Vars::getUID(const char* name, r::uid dflt) {
	if (has(name)) {
		auto an = get(name);
		if (an->type != AType::AUInt64) return dflt;
		return an->asUInt64();
	}
	return dflt;
}

const char* rd::Vars::getString(const char* name, const char* dflt) const {
	if (!has(name))	return (dflt==0)?emptyString: dflt;
	rd::Anon* meta = get(name);
	if (!meta) 	return (dflt == 0)?emptyString : dflt;
	if (meta->type != AType::AString) return emptyString;
	return meta->asString();
}

StrRef rd::Vars::getStr(const char* name) const {
	if (!has(name))	return StrRef(emptyString);
	rd::Anon* meta = get(name);
	if (!meta) return StrRef(emptyString);
	if (meta->type != AType::AString) return StrRef(emptyString);
	return StrRef(meta->asString());
}

r::u32 Vars::getU32(const char* name, r::u32 dflt) const {
	if (has(name)) {
		auto an = get(name);
		if (an->type != AType::AInt) return dflt;
		return (u32)an->asInt();
	}
	return dflt;
}

int Vars::getInt(const char* name, int dflt) const {
	if (has(name)) {
		auto an = get(name);
		if (an->type != AType::AInt) {
			if (an->type == AType::AFloat)
				return std::lrint(an->asFloat());
			if (an->type == AType::AInt64)
				return (int)an->asInt64();
			return dflt;
		}
		return an->asInt();
	}
	return dflt;
}

float Vars::getFloat(const char* name, float dflt) const {
	if (has(name)) {
		auto an = get(name);
		if (an->type != AType::AFloat) {
			if (an->type == AType::AInt)
                return (float)an->asInt();
            if (an->type == AType::ADouble)
                return (float)an->asDouble();
			return dflt;
		}
		return an->asFloat();
	}
	return dflt;
}

double Vars::getDouble(const char* name, double dflt) const {
    if (has(name)) {
        auto an = get(name);
        if (an->type != AType::ADouble) {
            if (an->type == AType::AInt)
                return (double)an->asInt();
            if (an->type == AType::AFloat)
                return (double)an->asFloat();
            return dflt;
        }
        return an->asDouble();
    }
    return dflt;
}

void* Vars::getPtr(const char* name, void* dflt) const {
	if (has(name)) {
		auto an = get(name);
		
		//size is unknown because well... probably don't need it
		if (an->type == AType::AByteBuffer) return an->asPtr();
		if (an->type == AType::AFloatBuffer) return an->asPtr();
		if (an->type == AType::AIntBuffer) return an->asPtr();

		//not a proper buffer?
		if (an->type == AType::AString) return an->asPtr();
		if (an->type != AType::APtr) return dflt;
		
		return an->asPtr();
	}
	return dflt;
}

bool Vars::getBuffer(const char* name, r::u8* out, int& _sz) const {
	if (has(name)) {
		auto an = get(name);
		if (an->type != AType::AByteBuffer) return true;
		r::u8* v = (r::u8*)an->asPtr();
		int sz = an->getSize();
		memcpy(out, v, sz);
		_sz = sz;
		return true;
	}
	return false;
}

bool Vars::getFloatBuffer(const char* name, std::vector<float>& out) const {
	if (has(name)) {
		auto an = get(name);
		if (an->type != AType::AFloatBuffer) return true;
		float* v = (float*)an->asPtr();
		int sz = an->getSize();
		for (int i = 0; i < sz; ++i)
			out.push_back(v[i]);
		return true;
	}
	return false;
}

r::Color Vars::getColor(const char* name, const r::Color& dflt ) const {
	if (has(name)) {
		auto an = get(name);
		if (an->type != AType::AFloatBuffer) return dflt;
		float * v = (float*)an->asPtr();
		return Color(v[0], v[1], v[2], v[3]);
	}
	return dflt;
}

r::opt<r::Vector4i> Vars::getVec4i(const char* name) const {
	if (has(name)) {
		auto an = get(name);
		if (an->type != AType::AInt && an->typeEx != ATypeEx::AVec4) return std::nullopt;
		Vector4i v;
		auto vf = an->asIntBuffer();
		v.x = vf[0];
		v.y = vf[1];
		v.z = vf[2];
		v.w = vf[3];
		return v;
	}
	return std::nullopt;
}

r::opt<r::Vector4> rd::Vars::getVec4(const char* name) const{
	if (!has(name)) return std::nullopt;

	auto an = get(name);
	if (an->type != AType::AFloat && an->typeEx != ATypeEx::AVec4) return std::nullopt;
	Vector4 v;
	auto vf = an->asFloatBuffer();
	v.x = vf[0];
	v.y = vf[1];
	v.z = vf[2];
	v.w = vf[3];
	return v;
}

r::Vector3 rd::Vars::getVec3(const char* name, Vector3 dflt) const{
	if (!has(name)) return dflt;
	auto an = get(name);
	if (an->type != AType::AFloat && an->typeEx != ATypeEx::AVec3) return dflt;
	Vector3 v;
	auto vf = an->asFloatBuffer();
	v.x = vf[0];
	v.y = vf[1];
	v.z = vf[2];
	return v;
}

r::Vector4 rd::Vars::getVec4(const char* name, Vector4 dflt) const{
	if (!has(name)) return dflt;
	auto an = get(name);
	if (an->type != AType::AFloat && an->typeEx != ATypeEx::AVec4) return dflt;
	Vector4 v;
	auto vf = an->asFloatBuffer();
	v.x = vf[0];
	v.y = vf[1];
	v.z = vf[2];
	v.w = vf[3];
	return v;
}

r::opt<r::Vector3> Vars::getVec3(const char* name) const {
	if (has(name)) {
		auto an = get(name);
		if (an->type != AType::AFloat && an->typeEx != ATypeEx::AVec3) 
			return std::nullopt;
		Vector3 v;
		auto vf = an->asFloatBuffer();
		v.x = vf[0];
		v.y = vf[1];
		v.z = vf[2];
		return v;
	}
	return std::nullopt;
}

r::opt<r::Vector2> Vars::getVec2(const char* name) const {
	if (has(name)) {
		auto an = get(name);
		if (an->type != AType::AFloat && an->typeEx != ATypeEx::AVec2)
			return std::nullopt;
		Vector2 v;
		auto vf = an->asFloatBuffer();
		v.x = vf[0];
		v.y = vf[1];
		return v;
	}
	return std::nullopt;
}

r::opt<r::Vector2i> Vars::getVec2i(const char* name) const {
	if (has(name)) {
		auto an = get(name);
		if (an->type != AType::AInt && an->typeEx != ATypeEx::AVec2)
			return std::nullopt;
		Vector2i v;
		auto vf = an->asIntBuffer();
		v.x = vf[0];
		v.y = vf[1];
		return v;
	}
	return std::nullopt;
}

r::opt<std::vector<void*>> Vars::getPtrArray(const char* name) const {
	if (has(name)) {
		auto an = get(name);
		if (an->type != AType::AByteBuffer && an->typeEx != ATypeEx::APtrArray) return std::nullopt;
		std::vector<void*> res;
		res.resize(an->getSize() / sizeof(void*));
		memcpy(res.data(), an->asByteBuffer(), an->getSize());
		return res;
	}
	return std::nullopt;
}

r::opt<std::vector<Vector3>> Vars::getVec3Array(const char* name) const {
	if (has(name)) {
		auto an = get(name);
		if (an->type != AType::AByteBuffer && an->typeEx != ATypeEx::AVec3Array) return std::nullopt;
		std::vector<Vector3> res;
		res.resize(an->getSize() / sizeof(Vector3));
		memcpy(res.data(), an->asByteBuffer(), an->getSize());
		return res;
	}
	return std::nullopt;
}

int Vars::getMemorySize() {
	int sz = sizeof(this);
	if (!head) return sz;
	return sz + head->getMemorySize();
}

int rd::Vars::count() const{
	if (!head) return 0;
	return head->countHierarchy();
}

bool rd::Vars::isEmpty() const{
	return head == 0;
}

const char* rd::Vars::TAGS_NAME = "_tags";
static const char* OLD_TAGS_NAME = "tags";

bool rd::Vars::hasTag(const char* name) const{
	if (!has(TAGS_NAME)) 
		if (!has(OLD_TAGS_NAME)) 
			return false;
		else 
			return rd::String::hasTag(getStr(OLD_TAGS_NAME), name);
	else 
		return rd::String::hasTag(getStr(TAGS_NAME), name);
}

void rd::Vars::setTag(const char* name, bool onOff) {
	rd::Anon* a = getOrCreate(TAGS_NAME);
	if(!a)
		a = getOrCreate(OLD_TAGS_NAME);
	if (onOff) {
		const char* cur = a->asString();
		if (rd::String::hasTag(cur, name))
			return;
		if (!cur || !*cur) {
			a->updateString(name);
		}
		else {
			Str256f ntag("%s,%s", cur,name );
			a->updateString(ntag.c_str());
		}
	}
	else {
		const char* asStr = a->asString();
		if (!asStr)
			return;
		if( 0 == strstr( asStr, name))//cannot find so no need to turn off
			return;

		std::string cur = a->asStdString();
		std::vector<std::string> allTags = rd::String::split(cur, ',');
		std::string sname = name;
		for (auto iter = allTags.begin(); iter != allTags.end(); ) {
			if (*iter == sname)
				iter = allTags.erase(iter);
			else 
				iter++;
		}
		a->updateString( rd::String::join( allTags,","));
	}
	
}

void rd::Vars::addTag(const char* name) {
	setTag(name, true);
}

void rd::Vars::removeTag(const char* name) {
	setTag(name, false);
}

Str rd::Vars::getTags(){
	if (!has(TAGS_NAME))
		return getStr(OLD_TAGS_NAME);
	return getStr(TAGS_NAME);
}


bool rd::Vars::isEqual(const rd::Vars& ov) const {
	if (!head && !ov.head)
		return true;
	else if (!head)
		return false;
	else if (!ov.head)
		return false;

	bool decide = true;
	traverse([&](const rd::Anon *a) {
		auto oa = ov.get(a->name.c_str());
		if (!a)
			decide = false;
		if (!oa)
			decide = false;
		if ( *oa != *a)
			decide = false;
	});
	return decide;
}


IntProperty::IntProperty(const char* fieldname, rd::Vars* v) {
	this->v = v;
	this->fieldname = fieldname;
}

int IntProperty::get() {
	return v->getInt(fieldname.c_str(), 0);
};

void IntProperty::set(int val) {
	v->set(fieldname.c_str(), val);
};