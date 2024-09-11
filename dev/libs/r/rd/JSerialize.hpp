#pragma once

#include <filesystem>
#include "1-files/File.h"
#include "4-ecs/JsonReflect.h"

#include "r2/Scene.hpp"
#include "r2/Bitmap.hpp"
#include "r2/Node.hpp"
#include "r2/Graphics.hpp"
#include "r2/Sprite.hpp"
#include "rd/Anon.hpp"
#include "rd/Dir.hpp"
#include "rs/Checksum.hpp"
#include "rd/Vars.hpp"
#include "rd/TileAnimPlayer.hpp"

namespace rd {
	struct EventRef;
	class Anon;

	class SerializationRegister {
	public:
		static SerializationRegister* me;

		std::vector<std::pair<int,std::function<void*()> >> data;

		SerializationRegister();
		
		void reg(int typeId, std::function<void*()> alloc) {
			data.push_back(std::pair(typeId,alloc));
		};
		
		template<typename T>
		T* create(int typeId) {
			for(auto &p : data)
				if (p.first == typeId)
					return (T*) p.second();
			return 0;
		};

		void* createRaw(int typeId);
	};

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
	static bool	jSerialize(T & t, const char* folder, const char* path, const char * rootName = nullptr) {
		Pasta::JWriter writer(0);
		writer.setDocument(new rapidjson::Document());
		writer.m_jvalue = writer.m_document;//why not...

		writer.visit(t, rootName);
		Pasta::FileMgr * mgr = Pasta::FileMgr::getSingleton();

		mgr->createDirectories((std::string("res/") +folder).c_str());
		Pasta::File * f = mgr->createFile( (std::string(folder) +"/"+ path ).c_str() );
		bool done = f->open(Pasta::File::FileAttribute::FA_WRITE);
		if (!f||!done) {
			rs::trace( Str256f("Error cannot create file! %s",path));
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
	static inline bool	jSerialize(T& t, const std::string& folder, const std::string& path, const char* rootName = nullptr) {
		return jSerialize(t, folder.c_str(), path.c_str(), rootName);
	};

	template<typename T>
	static inline bool	jSerialize(T& t, const Str& folder, const Str& path, const char* rootName = nullptr) {
		return jSerialize(t, folder.c_str(), path.c_str(), rootName);
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
			std::cout << "parse err:" << err << "\n";
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
	static bool	jDeserializeRaw(T& target, const char * path) {
		std::string res;
		bool ok = rs::File::read(path,res);
		if (!ok) return ok;
		bool deserOk = jDeserializeFromString(target, res);
		return deserOk;
	}

	static void prefetch(const char* folder, const char* path);

	template<typename T>
	static bool	jDeserialize(T & target, const char * folder, const char * path, const char* rootName = nullptr) {
		Pasta::FileMgr * mgr = Pasta::FileMgr::getSingleton();
		Str512f fullPath("%s/%s", folder, path);
		char* txt = 0;
		bool useCached = true;
		if (!rd::RscLib::fileCache.enabled) {
			std::string npath = mgr->convertResourcePath(fullPath.c_str());
			txt = (char*)mgr->load(npath);
			useCached = false;
		}
		else {
			txt = rd::RscLib::fileCache.getCachedText(fullPath.c_str());
			useCached = true;
		}
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
		if(!useCached)
			mgr->release(txt);
		return ok;
	};

	template<typename T>
	static bool	jDeserialize(T& target, const std::string& folder, const std::string& path, const char* rootName = nullptr) {
		return jDeserialize(target, folder.c_str(), path.c_str(), rootName);
	};
}

namespace Pasta {
	extern template void Pasta::JReflect::visit<>(rd::Anon& c, const char* name);
	extern template void JReflect::visit(rd::Vars& c, const char* name);
	extern template void JReflect::visit(r::Vector4& c, const char* name);
	extern template void JReflect::visit(r::Matrix44& c, const char* name);
	extern template void JReflect::visit(r::Vector3 &c, const char * name);
	extern template void JReflect::visit(r::Vector3i& c, const char* name);
	extern template void JReflect::visit(r::Vector3s& c, const char* name) ;
	extern template void JReflect::visit(r::Color &c, const char * name) ;
	extern template void JReflect::visit(Pasta::Color &c, const char * name);
	extern template void JReflect::visit(r2::Tri& c, const char* name);
	extern template void JReflect::visit<>(r2::Vertex& vtx, const char* name);

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
	DECL_SER_OPT( r::Color )
	


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
	extern template void JReflect::visit<>(r2::Tile& a, const char* name);
	extern template void Pasta::JReflect::visit<>(Str& str, const char* _name);

	template<> inline void Pasta::JReflect::visit(r2::Bounds& b, const char* _name) {
		if (_name) visitObjectBegin(_name);
		visit(b.xMin, "xmin");
		visit(b.xMax, "xmax");
		visit(b.yMin, "ymin");
		visit(b.yMax, "ymax");
		if (_name) visitObjectEnd(_name);
	};

	template<> inline void Pasta::JReflect::visit(rd::Dir& val, const char* _name) {
		visit((int&)val, _name);
	};

	template<> inline void JReflect::visit(rd::TilePackage& p, const char* name) {
		p.serialize(this, name);
	};

	template<> inline void JReflect::visit(qbool & val, const char* _name) {
		visit((int&)val, _name);
	};

	//template<> inline void JReflect::visit(rd::EventRef& ref, const char* _name) {
	//	ref.visit(this, _name);
	//};
	
}