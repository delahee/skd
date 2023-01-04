#pragma once

#if defined(_WIN64) && defined(DIRECTX_MATH_VERSION)
#include "DirectXPackedVector.h"
#define DX_HVEC
#endif

#include "PastaAssert.h"
#include "1-graphics/geo_vectors.h"
#include "1-graphics/FrameBuffer.h"
#include "1-graphics/Texture.h"
#include <optional>

namespace r {
	typedef Pasta::Matrix44				Matrix44;
	typedef Pasta::Matrix43				Matrix43;

	typedef Pasta::Vector2				Vector2;
	typedef Pasta::Vector3				Vector3;
	typedef Pasta::Vector2i				Vector2i;
	typedef Pasta::Vector3i				Vector3i;
	typedef Pasta::Vector4				Vector4;

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

	typedef r::Vector3					Triangle[3];

#ifdef DX_HVEC
	typedef HALF f16;
#else
	typedef float f16;
#endif

	//template<typename T>
	//using uptr = typename std::unique_ptr<T>;

	enum DIRECTION {
		UP = 1,
		DOWN = 2,
		LEFT = 4,
		RIGHT = 8,

		UP_LEFT = (1 | 4),
		UP_RIGHT = (1 | 8),

		DOWN_LEFT = (2 | 4),
		DOWN_RIGHT = (2 | 8),

		DIAG_TL = 16,
		DIAG_TR = 32,
		DIAG_BR = 64,
		DIAG_BL = 128,

		UP_DOWN = (1 | 2),//often aka vert center
		LEFT_RIGHT = (4 | 8),//often aka horiz center

		UP_DOWN_LEFT = (1 | 2 | 4),
		UP_DOWN_RIGHT = (1 | 2 | 8),
		UP_LEFT_RIGHT = (1 | 4 | 8),
		DOWN_LEFT_RIGHT = (2 | 4 | 8),
		TLDR = (1 | 2 | 4 | 8),

		NONE = ~0U
	};

	extern const char* dirToString(DIRECTION dir);
	extern std::vector<DIRECTION>	straightDirs;
	extern std::vector<DIRECTION>	allDirs;
	extern Vector3i					followDir(const Vector3i& v, DIRECTION dir);

	class Ref {
	public:
		Ref() { nbRef = 1; };
		Ref(const Ref&) { nbRef = 1; };

		virtual			~Ref() { PASTA_ASSERT_MSG(nbRef <= 0, "Unreleased object!"); }

		virtual int		incrRef() { return ++nbRef; }
		inline int		addRef() { return incrRef(); }
		inline int		acquire() { return incrRef(); }
		inline int		lock() { return incrRef(); }

		virtual void	decRef() { PASTA_ASSERT(nbRef > 0); if (nbRef > 0) { nbRef--; if (nbRef == 0) { delete this; } } };
		inline void		release() { decRef(); };

		inline int		refCount() const { return nbRef; }
	protected:
		int				nbRef = 0;
	};

	class IImWindow {
	public:
		virtual std::string	getName() = 0;
		virtual bool		isImOpened() = 0;
		virtual void		setImOpened(bool onOff) = 0;
		virtual bool		im() = 0;
	};

	class IImEntry {
	public:
		virtual bool		im() = 0;
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

namespace std{
	template<> struct hash<Str>	{
		std::size_t operator()(const Str & s) const noexcept{
			return hashStr(s.c_str());
		}
	};

	template<> struct hash<Pasta::Vector2i> {
		std::size_t operator()(const Vector2i& v) const noexcept {
			return v.x + 0x9e3779b9 * v.y;
		}
	};

	template<> struct hash<Pasta::Vector3i> {
		std::size_t operator()(const Vector3i& v) const noexcept {
			return v.x + 0x13371337 * v.y + 0x9e3779b9 * v.z;
		}
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

#include "Macros.hpp"




