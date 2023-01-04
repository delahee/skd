#pragma once


#include <string>
#include <vector>
#include <functional>
#include "r/Types.hpp"
namespace r2 {
	class Node;
	class BatchElem;
}
namespace rd {
	enum class AType : u32 {
		AVoid,
		AFloat,
		AInt,
		AString,

		AIntBuffer,
		AFloatBuffer,
		AByteBuffer,

		APtr,
		AInt64,
		AUInt64,
		ADouble,
	};

	enum class ATypeEx : u32 {
		AExVoid,
		
		AMat44,

		AVec2,
		AVec3,
		AVec4,
		AColor,

		ANodePtr,
		ABatchElemPtr,

		APtrArray,
		AVec3Array,

		ABool,
	};

	enum AFlags : u32 {
		AFL_OWNS_DATA = 1 << 0,

		AFL_IMGUI_CHANGING_COLOR = 1<<16,
	};

	//todo optimize me : pad at least 128 bits to pack float colors
	class Anon {
	public:
										Anon(const char * name = nullptr);
										Anon(Anon&&val);
										~Anon();

		std::string						name;

		rd::AType						type				= rd::AType::AVoid;
		rd::ATypeEx						typeEx				= rd::ATypeEx::AExVoid;

		rd::Anon*						sibling				= nullptr;
		rd::Anon*						child				= nullptr;

		void*							data				= nullptr;
#ifdef ENVIRONMENT32 //we need 64bit of storage
		int								_pad				= 0;
#endif

		bool							hasValue() { return type != AType::AVoid; }

		bool&							asBool();
		float&							asFloat();
		int& 							asInt()					;
		int *							asIntBuffer()			;
		float *							asFloatBuffer()			;
		std::vector<float>				getFloatBufferCopy()	;
		char*							asString()				;
		Str								asStr()					;
		std::string						asStdString()			;
		void *							asPtr();
		r2::Node*						asNodePtr();
		r2::BatchElem*					asBatchElemPtr();
		int64_t&						asInt64();
		uint64_t&						asUInt64();
		double&							asDouble();
		float*							asVertex3Buffer();
		r::Color&						asColor();

		void							reserve(int size);
		int								getSize();

		unsigned char *					asByteBuffer()			{ return (unsigned char *)data;					};

		rd::Anon*						mkVoid();
		rd::Anon*						mkInt(int v);
		rd::Anon*						mkBool(bool b);
		rd::Anon*						mkFloat(float v);
		rd::Anon*						mkMatrix44(float * v);
		rd::Anon*						mkVec2(const r::Vector2& v);
		rd::Anon*						mkVec3(const r::Vector3& v);
		rd::Anon*						mkVec4(const r::Vector4& v);
		rd::Anon*						mkColor(const r::Color& v);
		rd::Anon*						mkByteBuffer(const uint8_t * bytes, int size);
		rd::Anon*						mkFloatBuffer(const float * v, int nbFloats);
		rd::Anon*						mkVertex3Buffer(const float * v, int nbVertex3);
		rd::Anon*						mkIntBuffer(const int * v, int nbInts);
		rd::Anon*						mkString(const char * str);
		rd::Anon*						mkString(const std::string & v);
		rd::Anon*						mkPtr(void * ptr);
		rd::Anon*						mkNodePtr(r2::Node * ptr);
		rd::Anon*						mkBatchElemPtr(r2::BatchElem * ptr);
		rd::Anon*						mkInt64(int64_t v);
		rd::Anon*						mkUInt64(uint64_t v);
		rd::Anon*						mkDouble(double v);

		//returns whether operation was a success
		bool							setValueFromString(AType t, ATypeEx ate,const char * val);
		bool							setValueFromString(AType t, const char * val);
		void							updateString(const std::string& v);
		void							updateUInt64(r::u64 v);

		bool							isBuffer();

		void							setFloat(float val);
		void							setInt(int val);

		void							setPtr(void * ptr);
		void							setFloatBuffer(const float * v, int nbFloats);
		void							setVertex3Buffer(const float * v, int nbVertex3);

		// fill the anon with a pointer to the data directly, use with caution
		void							aliasFloatBuffer(float * v, int nbFloats);

		void							addChild(rd::Anon * c);
		void							addSibling(rd::Anon * c);
		int								countHierarchy()const;
		bool							isSorted();
		bool							isBool() { return type == AType::AInt && typeEx == ATypeEx::ABool; };
		bool							isInt() { return type == AType::AInt; };
		bool							isFloat() { return type == AType::AFloat; };
		rd::Anon*						clone( bool recursive = true ) const;

		bool							im(rd::Anon*& selfref);
		int								getByteSizeCapacity() { return byteSize; };
		
		rd::Anon*						getByName(const char * name);
		const rd::Anon*					getByName(const char * name) const;

		inline rd::Anon*				findByName(const char* name) { return getByName(name); };
		inline const rd::Anon*			findByName(const char* name) const { return getByName(name); };

		void							traverse( std::function<void(rd::Anon*)> f);
		std::string						toString() const;
		std::string						getValueAsString();

		rd::Anon*						destroy();
		rd::Anon*						destroyByName(const char * name);

		int								getMemorySize();
	public:
		static void						unitTest();

		static rd::Anon*				createInt(int val = 0, const char * name = nullptr);
		static rd::Anon*				createFloat(float val = 0.f, const char * name = nullptr);
		
	public:
		void							operator=(const Anon& rhs);

	protected:
		int								byteSize = 0;
		int								flags = 0;
		void							freeData();
	};
};

