#pragma once

#include "r/Color.hpp"
#include "rs/Reader.hpp"

namespace fmt {
	struct u8Color {
		uint8_t r = 0;
		uint8_t g = 0;
		uint8_t b = 0;

		r::Color toColor() {
			const float inv255 = 1.0f / 255.0f;
			return r::Color(r*inv255, g*inv255, b*inv255);
		}

		unsigned int toUInt() {
			return (r << 16) | (g << 8) | b;
		};

		unsigned int toUIntRGBA() {
			return (0xff<<24) | (b << 16) | (g << 8) | r;
		};
	};

	struct PhotoshopAct {
		vector<u8Color>	colors;//256 colors blocks at max
		int				colorCount = 0;
		int				transp = -1;

		eastl::vector<uint32_t> getPaletteRGBA() {
			eastl::vector<uint32_t> res;
			for (int i = 0; i < colorCount; ++i)
				res.push_back(colors[i].toUIntRGBA());
			return res;
		}
	};

	class PhotoshopActReader {
	public:
		static PhotoshopAct read(void * data) {
			PhotoshopAct a;
			a.colors.assign((u8Color *)data, ((u8Color *)data) + 256);
			uint8_t * dataPtr = (uint8_t *)data;
			dataPtr += 256 * 3;
			a.colorCount = rs::Reader::readU16_swapped(dataPtr);
			a.transp = rs::Reader::readU16_swapped(dataPtr);
			return a;
		};
	};
}