#pragma once

#include <cstdint>
#define _CRT_SECURE_DEPRECATE_MEMORY
#include <memory.h>

namespace rs {

	class Reader{
	public:

		static uint16_t readU16(uint8_t * & data) {
			uint16_t val = 0;
			memcpy(&val, data, 2);
			data += 2;
			return val;
		};

		static uint16_t changeEndianness(uint16_t val) {
			val = (((val >> 0) & 0xFF) << 8) | (((val >> 8) & 0xFF) << 0);
			return val;
		}

		static uint32_t changeEndianness(uint32_t val) {
			val = (((val >> 0) & 0xFF) << 24) | (((val >> 8) & 0xFF) << 16) | (((val >> 16) & 0xFF) << 8) | (((val >> 24) & 0xFF) << 0);
			return val;
		}

		static uint16_t readU16_swapped(uint8_t * & data) {
			uint16_t val = 0;
			memcpy(&val, data, 2);
			data += 2;
			val = changeEndianness(val);
			return val;
		};

		static uint32_t readU32(uint8_t *  & data) {
			uint32_t val = 0;
			memcpy(&val, data, 4);
			data += 4;
			return val;
		};

		static uint32_t readU32_swapped(uint8_t * & data) {
			uint32_t val = 0;
			memcpy(&val, data, 4);
			data += 4;
			val = changeEndianness(val);
			return val;
		};
	};
}