#pragma once

#include "r/Types.hpp"
#include <string>
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

	class RscLib {
	public:
		static bool					allowBitArrayProduction;

		/*file will stay in memory, release it asap*/
		static bool					hasText(const std::string &path);
		static char*				getText(const std::string& path);
		static void					releaseText(char* data);

		/*file will stay in memory, release it asap*/
		static void *				getBinary(const std::string &path, unsigned int * sz = 0);
		static void					releaseBinary(void * data);

		/*file will stay in memory, release it asap after copying/processing*/
		static JSONResource			getJSON(const std::string &path);
		static void					releaseJSON(JSONResource& rsc);

		/*file will stay in memory, release it asap*/
		static PLZResource			getPLZ(std::string path);
		static void					releasePLZ(PLZResource & rsc);

		/*file will stay in memory, release it asap*/
		static Pasta::TextureData*	getTextureData(const std::string & path);
		static void					releaseTextureData(Pasta::TextureData * data);

		static void					makeBitArray(const char* path, Pasta::TextureData* data);
		static rd::BitArray*		getBitArray(const char* path);

		static r::u32				getDecompressedSizeLZ(const char* src);
		static void					compressLZ(const char* src, r::u32 size, char* dst, r::u32& compSize);

		//inout decompSize in as max out out as effective size
		static void					decompressLZ(const char* src, r::u32 size, char* dst, r::u32& decompSize);

		static std::vector<TransientTexture>
									transientTex;
	};


}