#pragma once

#include <string>

#include "TileLib.hpp"

namespace rd{

	struct TPSlice {
		std::string name;
		int frame			=-1;
		int x				=-1;
		int y				=-1;
		int width			=-1;
		int height			=-1;
		int frameX			=-1;
		int frameY			=-1;
		int frameWidth		=-1;
		int frameHeight		=-1;
	};

	struct TPConf{
		bool treatFoldersAsPrefixes = false;
		r2::TexFilter filter = r2::TexFilter::TF_NEAREST;
		bool loadAllPages = true;
		bool skipSubFrames = false;
		bool keepTextureData = false;
	};

	class TexturePacker {
	public:
		/**load will just load the sheet straight away, data caching will be done at low level but serialization will always occur
		* honestly use the tilepicker
		*/
		static TileLib* load(const std::string & path, TPConf* conf = nullptr);
		static TileLib* parseXmlAndLoad(const std::string & path, const std::string & imgPath, TPConf* conf = nullptr);
		static TileLib* parseXmlAndLoad(const std::string & path, const std::string & imgPath, const TPConf & conf);
	};
}