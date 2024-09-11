#pragma once

#if defined(_WIN64) && defined(DIRECTX_MATH_VERSION)
#include "DirectXPackedVector.h"
#define DX_HVEC
#endif

#include "PastaAssert.h"
#include "1-graphics/geo_vectors.h"
#include "1-graphics/FrameBuffer.h"
#include "1-graphics/Texture.h"
#include "Str.h"
#include <optional>



namespace r {
	typedef Pasta::Matrix44				Matrix44;
	typedef Pasta::Matrix43				Matrix43;

	typedef Pasta::Vector2				Vector2;
	typedef Pasta::Vector3				Vector3;
	typedef Pasta::Vector2i				Vector2i;
	
	typedef Pasta::Vector3i				Vector3i;
	typedef Pasta::Vector4				Vector4;

	//glsl style vec because am bored sweetie poo
	typedef Pasta::Vector3i				vec3i;
	typedef Pasta::Vector3				vec3;
	typedef Pasta::Vector4				vec4;
	typedef Vector2i					vec2i;
	typedef Vector2						vec2;

	typedef Pasta::FrameBuffer			FrameBuffer;

	typedef Pasta::Texture				Texture;
	typedef Pasta::TransparencyType		TransparencyType;

	typedef std::uint8_t				u8;
	typedef std::uint16_t				u16;

	typedef std::uint32_t				u32;
	typedef std::uint64_t				u64;
	typedef std::uint64_t				uid;

	typedef std::int8_t					s8;
	typedef std::int16_t				s16;
	typedef std::int32_t				s32;
	typedef std::int64_t				s64;
	typedef std::int64_t				sid;

	typedef std::int32_t				c32;//type for charcodes ( ofc char on u8 is not enough and c++ doesn't have this )

	typedef Str							TKey;
	typedef r::Vector3					Triangle[3];

#ifdef DX_HVEC
	typedef HALF f16;
#else
	typedef float f16;
#endif

	enum class Type : int {
		None,
		Int,
		Float,
		Vec2,
		Vec3,
		Vec4,
		Vec2i,
		Vec3i,
		Vec4i,
		Color,
		Vars,
		Enum,
		String,
	};

	//template<typename T>
	//using uptr = typename std::unique_ptr<T>;

	class IImWindow {
	public:
		virtual				~IImWindow() {};
		virtual std::string	getName() = 0;
		virtual bool		isImOpened() = 0;
		virtual void		setImOpened(bool onOff) = 0;
		virtual bool		im() = 0;
	};

	class IImEntry {
	public:

		virtual				~IImEntry() {};
		virtual bool		im() = 0;
	};


	class Object {
	public:
		virtual ~Object() {};
	};

	enum qbool : int {
		False,
		True,
		Undetermined
	};
}

//remove this to reduce namespace pollution but requires lotta rewrites
//better remove some aliasing if poses a problem, box problems in rplatform if need be
using namespace r;

static inline std::size_t hashStr(const char * str ) {
	std::size_t hash = 7;
	while(*str){
		hash = hash * 31 + *str;
		str++;
	}
	return hash;
}

namespace eastl {
	template<> struct hash<Str> {
		inline std::size_t operator()(const Str& s) const noexcept {
			return hashStr(s.c_str());
		};
	};
}
namespace std{
	//see rs::Std for to_string(...)

	template<> struct hash<Str>	{
		inline std::size_t operator()(const Str& s) const noexcept {
			return hashStr(s.c_str());
		};
	};

	template<> struct hash<Pasta::Vector2i> {
		inline std::size_t operator()(const Vector2i& v) const noexcept {
			return v.x + 0x9e3779b9 * v.y;
		};
	};

	template<> struct hash<std::pair<vec2i,vec2i>> {
		inline std::size_t operator()(const std::pair<vec2i, vec2i>& v) const noexcept {
			auto& p0 = v.first;
			auto& p1 = v.second;
			return p0.x + 0x9e3779b9 * p0.y + p1.x * 0x13371337 + p1.y * (0x9e3779b9 ^ 0x13371337);
		};
	};

	template<> struct hash<Pasta::Vector3i> {
		inline std::size_t operator()(const Vector3i& v) const noexcept {
			return v.x + 0x13371337 * v.y + 0x9e3779b9 * v.z;
		};
	};

}



// Check windows
#if _WIN32 || _WIN64
#if _WIN64
#define ENVIRONMENT64
#else
#define ENVIRONMENT32
#endif
#endif

// Check GCC
#if __GNUC__
#if __x86_64__ || __ppc64__
#define ENVIRONMENT64
#else
#define ENVIRONMENT32
#endif
#endif

#include "r/Vector2d.hpp"
#include "r/Vector4i.hpp"

namespace r {
	typedef Vector2d					vec2d;
	typedef Vector4i					vec4i;
}

#include "Macros.hpp"



