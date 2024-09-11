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

	//add new values at the end
	enum class AType : int {
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

		ASharedPtr,
	};

	//add new values at the end
	enum class ATypeEx : int {
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
		AAgentPtr,
	};

	enum AFlags : int {
		AFL_OWNS_DATA = 1 << 0,
		AFL_IMGUI_CHANGING_COLOR = 1<<1,
		AFL_POOLED = 1<<2,
	};

	//todo optimize me : pad at least 128 bits to pack float colors
	class Anon {
	public:
										Anon(const char * name = nullptr);
										Anon(Anon&&val);
										~Anon();

		int								flags				= 0;
		rd::AType						type				= rd::AType::AVoid;
		rd::ATypeEx						typeEx				= rd::ATypeEx::AExVoid;

		int								byteSize			= 0;//avoid unsigned as overflow will create more error than what it tries to solve

		void* data = nullptr;
#ifdef ENVIRONMENT32 //we need 64bit of storage
		int								_pad = 0;
#endif

		rd::Anon*						sibling				= nullptr;
		rd::Anon*						child				= nullptr;
		std::string						name;

		bool							hasValue() const { return type != AType::AVoid; }

		bool&							asBool();
		float&							asFloat();
		int& 							asInt()					;
		int *							asIntBuffer()			;
		float *							asFloatBuffer()			;
		std::vector<float>				getFloatBufferCopy()	;
		char*							asString()				;
		Str								asStr()					;
		StrRef							asStrRef()				;
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
		rd::Anon*						mkAgentPtr(rd::Agent * ptr);
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
		bool							isString() { return type == AType::AString; };
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

		void							dispose();
		rd::Anon*						destroy();
		rd::Anon*						destroyByName(const char * name);

		int								getMemorySize();

		static rd::Anon*				fromPool(const char * name = 0);

		/*
		template<typename T>
		rd::Anon* mkSharedPtr(std::shared_ptr<T> ptr) {
			freeData();
			reserve(byteSize = sizeof(ptr));
			type = rd::AType::ASharedPtr;

			auto ref = (std::shared_ptr<T> *) &data;
			*ref = ptr;

			return this;
		};
		*/
	public:
		static void						unitTest();

		static rd::Anon*				createInt(int val = 0, const char * name = nullptr);
		static rd::Anon*				createFloat(float val = 0.f, const char * name = nullptr);
		
	public:
		void							operator=(const Anon& rhs);
		bool							operator==(const Anon& rhs)const;
		bool							operator!=(const Anon& rhs)const { return !(*this == rhs); };
	protected:
		void							freeData();
	};
};

