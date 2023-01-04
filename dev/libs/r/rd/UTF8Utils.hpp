#pragma once

#include <inttypes.h>
#include <cwchar>
#include <string>
#include <wchar.h>

namespace rd {

	typedef uint8_t UTF8Char;

	class UTF8Utils
	{
	public:

		static uint32_t				CountCharWidth(const UTF8Char & _Src);
		static uint32_t				StrLen(const UTF8Char * _Src);
		static const UTF8Char *		Advance(const UTF8Char * _Src, uint32_t & _Code);
		static uint32_t				GetLastBits(uint32_t _Code, uint32_t _Nth);

		//codepage is not modified
		static wchar_t *			utf8ToWchar(wchar_t * _Dest, const UTF8Char * _Src, uint32_t _MaxDestSize);
		
	};
}