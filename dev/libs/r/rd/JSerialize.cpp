#include "stdafx.h"
#include "JSerialize.hpp"

using namespace rd;
using namespace Pasta;

rd::SerializationRegister* rd::SerializationRegister::me = 0;

rd::SerializationRegister::SerializationRegister() {
	me = this;
}

void rd::prefetch(const char* folder, const char* path) {
	Pasta::FileMgr* mgr = Pasta::FileMgr::getSingleton();
	Str512f fullPath("%s/%s", folder, path);
	rd::RscLib::fileCache.getCachedText(fullPath.c_str());
};

void* rd::SerializationRegister::createRaw(int typeId) {
	for (auto& p : data)
		if (p.first == typeId)
			return p.second();
	return 0;
}

template<> void JReflect::visit(r2::Tile& a, const char* name) {
	if (name) visitObjectBegin(name);

	if (a.getTexture()) {
		std::string rscName = (a.getTexture()) ? a.getTexture()->getResourceName() : "";
		visit(rscName, "rscName");
		if (m_read) {
			//see if we can find it in tile sources
		}
	}

	visit(a.flags, "flags");

	visit(a.dx, "dx");
	visit(a.dy, "dy");

	visit(a.x, "x");
	visit(a.y, "y");

	visit(a.width, "width");
	visit(a.height, "height");

	visit(a.u1, "u1");
	visit(a.v1, "v1");
	visit(a.u2, "u2");
	visit(a.v2, "v2");
	if (name) visitObjectEnd(name);
};

template<> void JReflect::visit(Str& str, const char* _name) {
	std::string s;
	if (m_read) {
		visitString(s, _name);
		str = s;
	}
	else {
		s.assign(str.c_str(), str.length());
		visit(s, _name);
	}
};

template<> void JReflect::visit(rd::Anon& c, const char* name) {
	using namespace rd;
	bool newMethod = true;
	if (isReadMode()) {
		auto baseObject = JsonGet(m_jvalue, name);
		newMethod = baseObject->IsArray(); // format detection
	}

	if (newMethod) {
		u32 size = 1;
		rd::Anon* anon = &c;
		while (anon->sibling) { anon = anon->sibling; size++; }
		visitArrayBegin(name, size);
		anon = &c;
		for (u32 i = 0; i < size; i++) {
			visitIndexBegin(i);
			visitObjectBegin(nullptr);

			visit(anon->name, "name");
			visit((int&)anon->type, "type");

			if (isReadMode() || (isWriteMode() && (anon->typeEx != rd::ATypeEx::AExVoid)))
				visit((int&)anon->typeEx, "typeEx");

			int szBytes = anon->getByteSizeCapacity();
			visit(szBytes, "szBytes");

			if (m_read) {
				auto t = anon->type;
				auto te = anon->typeEx;
				switch (anon->type) {
				case AType::AVoid:
					break;
				case AType::AFloat:
					anon->mkFloat(0);
					break;
				case AType::AFloatBuffer:
					anon->mkFloatBuffer(nullptr, szBytes / sizeof(float));
					anon->typeEx = te;//restore type
					break;
				case AType::AIntBuffer:
					anon->mkIntBuffer(nullptr, szBytes / sizeof(int));
					anon->typeEx = te;//restore type
					break;
				case AType::AByteBuffer: {
					//reserve space
					anon->mkByteBuffer(nullptr, szBytes);
					anon->type = t;//restore type
					anon->typeEx = te;//restore type
					break;
				}
				case AType::AString:
					anon->mkByteBuffer(nullptr, szBytes);
					anon->type = t;//restore type
					anon->typeEx = te;//restore type
					break;
				case AType::AInt:
					anon->mkInt(0);
					break;

				case AType::AInt64:
					anon->mkInt64(0);
					break;
				case AType::AUInt64:
					anon->mkUInt64(0);
					break;
				default:
					break;
				}
			}

			switch (anon->type) {
			case AType::AVoid:
				break;

			case AType::AByteBuffer: {
				Pasta::u8* bb = (Pasta::u8*)anon->asByteBuffer();
				std::string b64;
				if (!isReadMode()) //write bytes
					b64 = rs::Encoding::encodeBase64Str(bb, anon->getByteSizeCapacity());
				int szB64 = b64.length();
				visit(szB64, "sz");
				visit(b64, "v");
				if (isReadMode())
					rs::Encoding::decodeBase64Buff(bb, szB64, b64);
				break;
			}
			case AType::AIntBuffer:
				visit(anon->asIntBuffer(), szBytes / 4, "val");
				break;
			case AType::AFloatBuffer:
				visit(anon->asFloatBuffer(), szBytes / 4, "val");
				break;
			case AType::AString:
				visitString(anon->asString(), szBytes, "val");
				break;
			case AType::AFloat:
				visit(anon->asFloat(), "val");
				break;
			case AType::AInt:
				visit(anon->asInt(), "val");
				break;
			case AType::AInt64:
				visit(anon->asInt64(), "val");
				break;
			case AType::AUInt64:
				visit(anon->asUInt64(), "val");
				break;
			case AType::ADouble:
				visit(anon->asDouble(), "val");
				break;
			default:
				break;
			}

			bool hasChild = anon->child != nullptr;

			if (isReadMode() || (isWriteMode() && hasChild))//modified so that we don't write the hasChild false to save a little space
				visit(hasChild, "hasChild");

			if (isReadMode() && hasChild)
				anon->child = rd::Anon::fromPool();
			if (anon->child)
				visit(*anon->child, "child");

			if (isReadMode() && i < size - 1)
				anon->sibling = rd::Anon::fromPool();
			anon = anon->sibling;
			visitObjectEnd(nullptr);
			visitIndexEnd();
		}
		visitArrayEnd(name);
	}
	else {
		if (name) visitObjectBegin(name);
		visit(c.name, "name");
		visit((int&)c.type, "type");


		visit((int&)c.typeEx, "typeEx");

		int szBytes = c.getByteSizeCapacity();
		visit(szBytes, "szBytes");

		if (m_read) {
			auto t = c.type;
			auto te = c.typeEx;
			switch (c.type) {
			case AType::AVoid:
				break;
			case AType::AFloat:
				c.mkFloat(0);
				break;
			case AType::AFloatBuffer:
				c.mkFloatBuffer(nullptr, szBytes / sizeof(float));
				c.typeEx = te;//restore type
				break;
			case AType::AIntBuffer:
				c.mkIntBuffer(nullptr, szBytes / sizeof(int));
				c.typeEx = te;//restore type
				break;
			case AType::AByteBuffer:
			case AType::AString:
				c.mkByteBuffer(nullptr, szBytes);
				c.type = t;//restore type
				c.typeEx = te;//restore type
				break;
			case AType::AInt:
				c.mkInt(0);
				break;

			case AType::AInt64:
				c.mkInt64(0);
				break;
			case AType::AUInt64:
				c.mkUInt64(0);
				break;
			default:
				break;
			}
		}

		switch (c.type) {
		case AType::AVoid:
			break;

		case AType::AByteBuffer: {
			Pasta::u8* bb = (Pasta::u8*)c.asByteBuffer();
			visit(bb, szBytes, "val");
			break;
		}
		case AType::AIntBuffer:
			visit(c.asIntBuffer(), szBytes / 4, "val");
			break;
		case AType::AFloatBuffer:
			visit(c.asFloatBuffer(), szBytes / 4, "val");
			break;
		case AType::AString:
			visitString(c.asString(), szBytes, "val");
			break;
		case AType::AFloat:
			visit(c.asFloat(), "val");
			break;
		case AType::AInt:
			visit(c.asInt(), "val");
			break;
		case AType::AInt64:
			visit(c.asInt64(), "val");
			break;
		case AType::AUInt64:
			visit(c.asUInt64(), "val");
			break;
		case AType::ADouble:
			visit(c.asDouble(), "val");
			break;
		default:
			break;
		}

		bool hasSibling = c.sibling != nullptr;
		visit(hasSibling, "hasSibling");
		if (isReadMode() && hasSibling)
			c.sibling = rd::Anon::fromPool();
		if (c.sibling)
			visit(*c.sibling, "sibling");

		bool hasChild = c.child != nullptr;
		visit(hasChild, "hasChild");
		if (isReadMode() && hasChild)
			c.child = rd::Anon::fromPool();
		if (c.child)
			visit(*c.child, "child");

		if (name) visitObjectEnd(name, true);
	}
};

template<> void JReflect::visit(rd::Vars& c, const char* name) {
	c.serialize(this, name);
}

template<> void JReflect::visit(r::Vector4& c, const char* name) {
	if (name) visitObjectBegin(name);
	visitFloat(c.x, "x");
	visitFloat(c.y, "y");
	visitFloat(c.z, "z");
	visitFloat(c.w, "w");
	if (name) visitObjectEnd(name, true);
};

template<> void JReflect::visit(r::Matrix44& c, const char* name) {
	visitObjectBegin(name);
	visit(c.ptr(), 16, "values");
	visitObjectEnd(name, true);
};


template<> void JReflect::visit(r::Vector3& c, const char* name) {
	visitObjectBegin(name);
	visitFloat(c.x, "x");
	visitFloat(c.y, "y");
	visitFloat(c.z, "z");
	visitObjectEnd(name, true);
};

template<> void JReflect::visit(r::Vector3i& c, const char* name) {
	visitObjectBegin(name);
	visit(c.x, "x");
	visit(c.y, "y");
	visit(c.z, "z");
	visitObjectEnd(name, true);
};

template<> void JReflect::visit(r::Vector3s& c, const char* name) {
	if (name) visitObjectBegin(name);
	visit(c.x, "x");
	visit(c.y, "y");
	visit(c.z, "z");
	if (name) visitObjectEnd(name, true);
};

template<> void JReflect::visit(r::Color& c, const char* name) {
	if (name) visitObjectBegin(name);
	visitFloat(c.r, "r");
	visitFloat(c.g, "g");
	visitFloat(c.b, "b");
	visitFloat(c.a, "a");
	if (name) visitObjectEnd(name, true);
};

template<> void JReflect::visit(Pasta::Color& c, const char* name) {
	if (name) visitObjectBegin(name);
	visitFloat(c.r, "r");
	visitFloat(c.g, "g");
	visitFloat(c.b, "b");
	visitFloat(c.a, "a");
	if (name) visitObjectEnd(name, true);
};

template<> void JReflect::visit(r2::Tri& c, const char* name) {
	if (name) visitObjectBegin(name);
	visit(c.cooPtr(), 3 * 4, "coo");
	visit(c.colorPtr(), 3 * 4, "col");
	if (name) visitObjectEnd(name);
};

template<> void JReflect::visit(r2::Vertex& c, const char* name) {
	if (name) visitObjectBegin(name);
	visit(c.pos.ptr(), 3, "pos");
	visit(c.uv.ptr(), 2, "uv");
	visit(c.col.ptr(), 4, "col");
	if (name) visitObjectEnd(name);
};