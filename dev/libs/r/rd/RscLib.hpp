#pragma once

#include <string>

#include "r/Types.hpp"

#include "1-files/FileMgr.h"
#include "1-json/jsoncpp/json.h"

namespace rd { class BitArray; }

namespace rd {
	struct JSONResource {
		void *			data = nullptr;
		Json::Value *	root = nullptr;
	};

	struct PLZResource {
		void *			uncompressedData	= nullptr;
		unsigned int	uncompressedSize	= 0;
		void *			data				= nullptr;
	};

	struct TransientTexture{
		r::u32				crc	= 0;
		Pasta::TextureData* data = 0;
		Pasta::Texture*		tex = 0;
	};

	struct Buffer {
		void*			ptr = 0;
		r::s64			size = 0;
		r::s64			ofs = 0;
		int				nbHit = 0;

		void	reset();
		void	dealloc();
		void*	base() const;
	};

	class MemCache {
	public:
		//when not enabled let the fs do its job but retain the data
		bool								enabled = false;
		std::unordered_map<Str, Buffer>		data;
		bool								has(const char * path);
		Buffer*								get(const char* path);
		void								set(const char* path, const Buffer& data);
		void								prefetchFile(const char* path);
		char*								getCachedText(const char* path);
		bool								getCachedFile(const char* path, Str& dest);

		void								im();
	};

	class RscLib {
	public:
		static bool					allowBitArrayProduction;

		//not multithread safe, uses a named cache
		static bool					has(const char* path);

		//multithread safe
		static bool					has(const std::string& path);

		/*file will stay in memory, release it asap*/
		static bool					hasText(const char* path);
		static char*				getText(const char* path);
		static void					releaseText(char* data);

		/*file will stay in memory, release it asap*/
		static void *				getBinary(const char * path, r::s64 * sz = 0);
		static void					releaseBinary(void * data);

		/*file will stay in memory, release it asap after copying/processing*/
		static JSONResource			getJSON(const std::string &path);
		static void					releaseJSON(JSONResource& rsc);

		/*file will stay in memory, release it asap*/
		static PLZResource			getPLZ(std::string path);
		static void					releasePLZ(PLZResource & rsc);

		/*file will stay in memory, release it asap*/
		static Pasta::TextureData*	getTextureData(const char* path);
		static Pasta::TextureData*	getTextureData(const std::string & path);
		static void					releaseTextureData(Pasta::TextureData * data);

		static void					makeBitArray(const char* path, Pasta::TextureData* data);
		static rd::BitArray*		getBitArray(const char* path);

		static r::u32				getDecompressedSizeLZ(const char* src);


		static void					compressLZ(const char* src, r::u32 size, char* dst, r::u32& compSize);
		static void					compressLZ(const rd::Buffer& src, rd::Buffer & dst);
		static void					compressLZ(const std::string & src, rd::Buffer & dst);

		//inout decompSize in as max out out as effective size
		static void					decompressLZ(const char* src, r::u32 size, char* dst, r::u32& decompSize);

		static std::vector<TransientTexture>
									transientTex;

		static MemCache				fileCache;
	};


}