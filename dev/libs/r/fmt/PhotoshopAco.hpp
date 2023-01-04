#pragma once

#include <vector>
#include "rs/Reader.hpp"

namespace fmt{

	struct PhotoshopColorV1 {
		uint16_t	colorSpace;
		uint8_t		colorValue[8];
	};

	struct PhotoshopColorV2 {
		std::string			name;
		PhotoshopColorV1	col;
	};

	struct PhotoshopAcoData {
		int								version = 1;//2 bytes
		int								colorCount = 0;//2 bytes color count
		std::vector<PhotoshopColorV1>	colorDataV1;

		int								version2 = 2;//2 bytes
		int								colorCount2 = 0;
		std::vector<PhotoshopColorV2>	colorDataV2;
	};

	class PhotoshopAco {

		//not the fastest way but the most failsafe way
		static PhotoshopAcoData read(void * data) {
			PhotoshopAcoData acoData;
			uint8_t * src = (uint8_t*)data;
			acoData.version = (int)rs::Reader::readU16(src);
			acoData.colorCount = (int)rs::Reader::readU16(src);
			for (int i = 0; i < acoData.colorCount; ++i) {
				PhotoshopColorV1 c;

				memcpy(&c, src, 10);
				src += 10;

				acoData.colorDataV1.push_back(c);
			}
			acoData.version2 = (int)rs::Reader::readU16(src);
			acoData.colorCount2 = (int)rs::Reader::readU16(src);
			return acoData;
		};
	};
}