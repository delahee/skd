#pragma once

#include "r/Types.hpp"

class Checksum
{
public:
	static void			Initialize();	// automatically called the first time you ask for a checksum

	// CRC32
	static r::u32		CRC32(const void* _Buffer, r::u32 _Size);

	// Adler32 (faster, particularly with large buffers)
	static r::u32		Adler32(const void* _Buffer, r::u32 _Size);
};

namespace rs {
	class Encoding {
	public:
		static std::vector<r::u8>	encodeBase64Vec(const r::u8* bytes, u32 size);
		static std::string			encodeBase64Str(const r::u8* bytes, u32 size);

		static void					decodeBase64Vec(std::vector<char>& _out, const std::string& _in);
		static void					decodeBase64Vec(std::vector<r::u8>& _out, const std::string& _in);
		static void					decodeBase64Buff(r::u8 * _out, int maxSize, const std::string& _in);
	};
}

