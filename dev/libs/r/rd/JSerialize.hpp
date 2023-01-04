#pragma once

#include <ctime>
#include <string>
#include <chrono>
#ifdef PASTA_WIN
#include <filesystem>
#endif

#include "4-ecs/JsonReflect.h"
#include "1-files/FileMgr.h"
#include "1-files/File.h"
#include "r2/Scene.hpp"
#include "r2/Bitmap.hpp"
#include "r2/Node.hpp"
#include "r2/Sprite.hpp"
#include "Anon.hpp"

#include "rs/Checksum.hpp"
#include <optional>

namespace rs {

	template<typename T>
	static std::vector<char> jSerializeToMemory(T& t ) {
		Pasta::JWriter writer(0);
		
		writer.setDocument(new rapidjson::Document());
		writer.m_jvalue = writer.m_document;//why not...

		writer.visit(t, nullptr);

		rapidjson::StringBuffer buf;
		writer.stringify(buf, true);

		return std::vector<char>(buf.GetString(), buf.GetString() + buf.GetSize());
	};

#if PASTA_WIN
	template<typename T>
	static bool	jSerializeRaw(T& t, const std::string& path, const char* rootName = nullptr) {
		std::filesystem::path lpath = path;
		Pasta::FileMgr::getSingleton()->createDirectories(lpath.root_directory().u8string().c_str());
		FILE* f = nullptr;
		fopen_s(&f, path.c_str(), "wb");
		bool done = f != nullptr;
		if (!done) {
			trace("Error cannot create file! " + path);
			return false;
		}
		std::vector<char> res = jSerializeToMemory(t);
		const char* ptr = res.data();
		fwrite(ptr, res.size(), 1, f);
		fflush(f);
		fclose(f);
		return true;
	}
#endif

	//serialization facilities for editors and stuff like that
	//saves in the game src folder, so not really for game saves
	template<typename T>
	static bool	jSerialize(T & t, const std::string & folder, const std::string & path, const char * rootName = nullptr) {
		Pasta::JWriter writer(0);
		writer.setDocument(new rapidjson::Document());
		writer.m_jvalue = writer.m_document;//why not...

		writer.visit(t, rootName);
		Pasta::FileMgr * mgr = Pasta::FileMgr::getSingleton();

		mgr->createDirectories(folder.c_str());
		Pasta::File * f = mgr->createFile( (folder +"/"+ path ).c_str() );
		bool done = f->open(Pasta::File::FileAttribute::FA_WRITE);
		if (!f||!done) {
			trace("Error cannot create file! " + path);
			return false;
		}
		rapidjson::StringBuffer buf;
		writer.stringify(buf, true);
		f->writeRawText(buf.GetString());
		f->flush();
		f->close();
		return true;
	};

	template<typename T>
	static std::string jSerializeToString(T & t, const char * name=0) {
		Pasta::JWriter writer(0);
		writer.setDocument(new rapidjson::Document());
		writer.m_jvalue = writer.m_document;//why not...
		if (name) writer.visitObjectBegin(name);
		writer.visit(t, 0);
		if (name) writer.visitObjectEnd(name,false);
		rapidjson::StringBuffer buf;
		writer.stringify(buf,false);
		return buf.GetString();
	};
	
	template<typename T>
	static bool jDeserializeFromString(T& target, const std::string& txt, const char* name = 0) {
		Pasta::JReader reader(0);
		reader.setDocument(new rapidjson::Document());
		bool error = reader.m_document->Parse(txt)
			.HasParseError();
		bool ok = false;
		if (error) {
			ok = false;
			auto err = reader.m_document->GetParseError();
			std::cout << "parse err:" << err << std::endl;
		}
		else {
			reader.m_jvalue = reader.m_document;
			if (name) reader.visitObjectBegin(name);
			reader.visit(target,0);
			if (name) reader.visitObjectEnd(name,false);
			ok = true;
		}
		return ok;
	};

	template<typename T>
	static bool	jDeserializeRaw(T& target, const std::string& path) {
		std::string res;
		bool ok = rs::File::read(path,res);
		if (!ok) return ok;
		bool deserOk = jDeserializeFromString(target, res);
		return deserOk;
	}

	template<typename T>
	static bool	jDeserialize(T & target, const std::string & folder, const std::string & path, const char* rootName = nullptr) {
		Pasta::FileMgr * mgr = Pasta::FileMgr::getSingleton();
		mgr->createDirectories(folder.c_str());
		std::string npath = mgr->convertResourcePath( (folder + "/" + path).c_str() );
		char * txt = (char*)mgr->load(npath);
		bool ok = false;
		if (txt) {
			Pasta::JReader reader(0);
			reader.setDocument(new rapidjson::Document());

			auto& doc = reader.m_document->Parse(txt);
			bool error = doc.HasParseError();
			
			if (error) ok = false;
			else {
				reader.m_jvalue = reader.m_document;//why not...
				reader.visit(target, rootName);
				ok = true;
			}
		}
		mgr->release(txt);
		return ok;
	};
}

namespace Pasta {
	template<> inline void JReflect::visit(rd::Vars& c, const char* name) {
		c.serialize(this, name);
	}

	template<> inline void JReflect::visit(rd::Anon &c, const char * name) {
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
					case AType::AByteBuffer:{
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
						b64 = rs::Encoding::encodeBase64Str(bb,anon->getByteSizeCapacity());
					int szB64 = b64.length();
					visit(szB64, "sz");
					visit(b64, "v");
					if(isReadMode())
						rs::Encoding::decodeBase64Buff( bb, szB64, b64);
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
				visit(hasChild, "hasChild");
				if (isReadMode() && hasChild)
					anon->child = new rd::Anon();
				if (anon->child)
					visit(*anon->child, "child");


				if (isReadMode() && i < size - 1)
					anon->sibling = new rd::Anon();
				anon = anon->sibling;
				visitObjectEnd(nullptr);
				visitIndexEnd();
			}
			visitArrayEnd(name);
		} else {
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
						c.mkByteBuffer(nullptr,szBytes);
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
					visit(c.asIntBuffer(), szBytes/4, "val");
					break;
				case AType::AFloatBuffer:
					visit(c.asFloatBuffer(), szBytes/4, "val");
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
				c.sibling = new rd::Anon();
			if (c.sibling) 
				visit(*c.sibling, "sibling");

			bool hasChild = c.child != nullptr;
			visit(hasChild, "hasChild");
			if (isReadMode() && hasChild)
				c.child = new rd::Anon();
			if (c.child)
				visit(*c.child, "child");

			if (name) visitObjectEnd(name, true);
		}
	};

	template<> inline void JReflect::visit(r::Vector4& c, const char* name) {
		if (name) visitObjectBegin(name);
		visitFloat(c.x, "x");
		visitFloat(c.y, "y");
		visitFloat(c.z, "z");
		visitFloat(c.w, "w");
		if (name) visitObjectEnd(name, true);
	};

	template<> inline void JReflect::visit(r::Matrix44& c, const char* name) {
		visitObjectBegin(name);
		visit(c.ptr(),16,"values");
		visitObjectEnd(name, true);
	};


	template<> inline void JReflect::visit(r::Vector3 &c, const char * name) {
		if (name) visitObjectBegin(name);
		visitFloat(c.x, "x");
		visitFloat(c.y, "y");
		visitFloat(c.z, "z");
		if (name) visitObjectEnd(name, true);
	};
	
	template<> inline void JReflect::visit(r::Vector3i& c, const char* name) {
		if (name) visitObjectBegin(name);
		visit(c.x, "x");
		visit(c.y, "y");
		visit(c.z, "z");
		if (name) visitObjectEnd(name, true);
	};

	template<> inline void JReflect::visit(r::Color &c, const char * name) {
		if (name) visitObjectBegin(name);
		visitFloat(c.r, "r");
		visitFloat(c.g, "g");
		visitFloat(c.b, "b");
		visitFloat(c.a, "a");
		if (name) visitObjectEnd(name,true);
	};

	template<> inline void JReflect::visit(Pasta::Color &c, const char * name) {
		if (name) visitObjectBegin(name);
		visitFloat(c.r, "r");
		visitFloat(c.g, "g");
		visitFloat(c.b, "b");
		visitFloat(c.a, "a");
		if (name) visitObjectEnd(name, true);
	};

	template<> inline void JReflect::visit(r2::Tri&c, const char * name) {
		if (name) visitObjectBegin(name);
		visit(c.cooPtr(), 3 * 3, "coos");
		visit(c.colorPtr(), 3 * 4, "colors");
		if (name) visitObjectEnd(name);
	};

/*
	template<> 
	inline void JReflect::visit(std::optional<Vector2>& c, const char* name) {
		if (name) visitObjectBegin(name); 
		bool hasVal = (c != std::nullopt); 
		visit(hasVal, "hv"); 
		if (hasVal) {
			if( !isReadMode())
				visit(*c, "v");//write it
			else{
				Vector2 val = {};//use a proxy for assign
				visit(val, "v");
				c = val;
			} 
		}
		if (name) visitObjectEnd(name); 
	};
	*/

#define DECL_SER_ENUM( ty ) \
	template<> \
	inline void Pasta::JReflect::visit(ty & c, const char* name) { \
		visit((int&) c, name); \
	} \

#define DECL_SER_OPT( ty ) \
	template<> \
	inline void Pasta::JReflect::visit(std::optional<ty> & c, const char* name) { \
		if (name) visitObjectBegin(name); \
		bool hasVal = (c != std::nullopt); \
		visit(hasVal, "hv"); \
		if(hasVal) \
			if( !isReadMode()) \
				visit(*c, "v"); \
			else { \
				ty val; \
				visit(val, "v"); \
				c = val; \
			} \
		if (name) visitObjectEnd(name); \
	}; \

	DECL_SER_OPT( bool )
	DECL_SER_OPT( int )
	DECL_SER_OPT( float )
	DECL_SER_OPT( double )
	DECL_SER_OPT( Vector2 )
	DECL_SER_OPT( Vector3 )
	DECL_SER_OPT( Vector4 )


#define DECL_SER_OPT_ENUM( ty ) \
	template<> \
	inline void Pasta::JReflect::visit(std::optional<ty> & c, const char* name) { \
		if (name) visitObjectBegin(name); \
		bool hasVal = (c != std::nullopt); \
		visit(hasVal, "hv"); \
		if(hasVal) \
			if( !isReadMode()) \
				visit( (int&) *c, "v"); \
			else { \
				ty val; \
				visit((int&) val, "v"); \
				c = val; \
			} \
		if (name) visitObjectEnd(name); \
	}; \

	template<> inline void JReflect::visit(rd::TileAnimPlayer&p, const char * name) {
		if (name) visitObjectBegin(name);
		//visit(p.needUpdates, "needUpdates");
		visit(p.destroyed, "destroyed");
		visit(p.isPlaying, "isPlaying");
		visit(p.frameRate, "frameRate");
		visit(p.speed, "speed");
		//lib and sprite will be bound from parent
		if (name) visitObjectEnd(name);
	};

	//should never really exist hu ?
	//template<> inline void JReflect::visit(rd::FrameData&p, const char * name);
	

	template<> inline void JReflect::visit(rd::TileAnim&a, const char * name) {
		if (name) visitObjectBegin(name);
		visit(a.cursor, "cursor");
		visit(a.plays, "plays");
		visit(a.speed, "speed");
		visit(a.paused, "paused");
		//todo from where you are!
		//visit(a.libName, "libName");
		//todo from where you are!
		//visit(a.groupName, "groupName");
		visit(a.curFrameCount, "curFrameCount");
		//lib and sprite will be bound from parent
		if (name) visitObjectEnd(name);
	};

	//should probably hint at metadata or tile retrieval thing around here
	template<> inline void JReflect::visit(r2::Tile& a, const char* name) {
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

	template<> inline void Pasta::JReflect::visit(r2::Bounds& b, const char* _name) {
		visit(b.xMin, "xmin");
		visit(b.xMax, "xmax");
		visit(b.yMin, "ymin");
		visit(b.yMax, "ymax");
	};

	template<> inline void Pasta::JReflect::visit(Str & str, const char* _name) {
		std::string s;
		if (m_read) {
			visit(s, _name);
			str = s;
		}
		else {
			s.assign(str.c_str(), str.length());
			visit(s, _name);
		}
	};

	template<> inline void Pasta::JReflect::visit(DIRECTION& val, const char* _name) {
		visit((int&)val, _name);
	};

}