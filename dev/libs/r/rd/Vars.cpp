#include "stdafx.h"
#include "Vars.hpp"
#include "rd/JSerialize.hpp"

using namespace std;
using namespace r;
using namespace rd;

static const char* const emptyString  = "";
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

void Vars::merge(Vars& other) {
	vector<const char*> lsNames; 
	other.traverse([&lsNames](rd::Anon* r) {
		lsNames.push_back(r->name.c_str());
	});

	for (const char* name : lsNames)
		destroy(name);

	this->operator+=(other);
}

void Vars::loadWithFilter(Vars& other, std::vector<string>& varFilters) {
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
	if (head) delete head;
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

rd::Anon* Vars::get(const char* name) {
	return (!head) ? nullptr : head->getByName(name);
}

rd::Anon* Vars::get(const char* name) const{
	return (!head) ? nullptr : head->getByName(name);
}

rd::Anon* Vars::getOrCreate(const char* _name) {
	if (!head) {
		head = new rd::Anon(_name);
		return head;
	}

	rd::Anon* an = head->getByName(_name);
	if (an) return an;
	an = new rd::Anon(_name);
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
	rd::Anon* meta = getOrCreate(name);
	meta->mkByteBuffer(content, nbBytes);
	return meta;
}

rd::Anon* Vars::set(const char* name, const float* content, int nbFloats) {
	rd::Anon* meta = getOrCreate(name);
	meta->mkFloatBuffer(content, nbFloats);
	return meta;
}

rd::Anon * rd::Vars::set(const char * name, const Vector3* content, int nbVec3) {
	rd::Anon* meta = getOrCreate(name);
	meta->mkByteBuffer(nullptr, nbVec3 * sizeof(Vector3));
	meta->typeEx = ATypeEx::AVec3Array;
	for (int i = 0; i < nbVec3; i++)
		((Vector3*)meta->data)[i] = content[i];
	return meta;
}

rd::Anon * rd::Vars::set(const char * name, const void** content, int nbPtr) {
	rd::Anon* meta = getOrCreate(name);
	meta->mkByteBuffer(nullptr, nbPtr * sizeof(void*));
	meta->typeEx = ATypeEx::APtrArray;
	for (int i = 0; i < nbPtr; i++)
		((void**)meta->data)[i] = (void*)content[i];
	return meta;
}

rd::Anon* Vars::set(const char* name, const char* content) {
	rd::Anon* meta = getOrCreate(name);
	meta->mkString(content);
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
	meta->mkFloat(content);
	return meta;
}

rd::Anon* Vars::set(const char* name, double content) {
	rd::Anon* meta = getOrCreate(name);
	meta->mkDouble(content);
	return meta;
}

rd::Anon* Vars::set(const char* name, int content) {
	rd::Anon* meta = getOrCreate(name);
	meta->mkInt(content);
	return meta;
}

rd::Anon* Vars::set(const char* name, bool content) {
	rd::Anon* meta = getOrCreate(name);
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

rd::Anon* Vars::set(const char* name, void* content) {
	rd::Anon* meta = getOrCreate(name);
	meta->mkPtr(content);
	return meta;
}

rd::Anon* rd::Vars::set(const char* name, const r::Color& content) {
	return set(name,content.ptr(), 4);
}

void Vars::incr(const char* name) {
	if (!has(name))
		set(name, 0);
	else
		set(name, (get(name)->asInt() + 1));
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


bool rd::Vars::im(bool showAsHeader) {
	using namespace ImGui;

	bool changed = false;
	PushID(this);

	bool open = showAsHeader ? 
		ImGui::CollapsingHeader(ICON_MD_STICKY_NOTE_2 " Metadata") : 
		ImGui::TreeNode(ICON_MD_STICKY_NOTE_2 " Metadata");
	if (open) {
		if (showAsHeader) Indent();
		if (head)
			changed |= r2::Im::metadata(head);
		else {
			Text("No metadata on record.");
			if (Button("Create")) {
				head = new Anon();
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
		if (head) delete head;
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
		head = new Anon();
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
	
	add(val->clone());
	return true;
}

void rd::Vars::load(const Vars& v){
	dispose();
	if (v.head)
		head = v.head->clone();
}

void Vars::dispose() {
	while (head) 
		head = head->destroy();
}

bool Vars::getBool(const char* name, bool dflt) {
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
	if (!has(name))	return dflt;
	rd::Anon* meta = get(name);
	if (!meta) return dflt;
	if (meta->type != AType::AString) return 0;
	return meta->asString();
}

StrRef rd::Vars::getStr(const char* name) const {
	if (!has(name))	return StrRef(emptyString);
	rd::Anon* meta = get(name);
	if (!meta) return StrRef(emptyString);
	if (meta->type != AType::AString) return 0;
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
			if (an->type == AType::AInt)
				return std::lrint(an->asFloat());
			else 
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
			return dflt;
		}
		return an->asFloat();
	}
	return dflt;
}

void* Vars::getPtr(const char* name, void* dflt) const {
	if (has(name)) {
		auto an = get(name);
		if (an->type != AType::APtr) return dflt;
		return an->asPtr();
	}
	return dflt;
}

r::Color Vars::getColor(const char* name, const r::Color& dflt ) const {
	if (has(name)) {
		auto an = get(name);
		if (an->type != AType::APtr) return dflt;
		float * v = (float*)an->asPtr();
		return Color(v[0], v[1], v[2], v[3]);
	}
	return dflt;
}

r::opt<r::Vector3> Vars::getVec3(const char* name) const {
	if (has(name)) {
		auto an = get(name);
		if (an->type != AType::AFloat && an->typeEx != ATypeEx::AVec3) return std::nullopt;
		Vector3 v;
		auto vf = an->asFloatBuffer();
		v.x = vf[0];
		v.y = vf[1];
		v.z = vf[2];
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

int rd::Vars::count() const
{
	if (!head) return 0;
	return head->countHierarchy();
}
