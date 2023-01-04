#include "stdafx.h"

#include "UTF8Utils.hpp"

namespace rd {
	const UTF8Char s_BOM[] =
	{
		0xEF,
		0xBB,
		0xBF,
		0
	};

	uint32_t UTF8Utils::CountCharWidth(const UTF8Char & _Src)
	{
		uint32_t l_Temp = 1;
		if (_Src & (1 << 7))
		{
			if (_Src & (1 << 6))
			{
				l_Temp++;
				if (_Src & (1 << 5))
				{
					l_Temp++;
					if (_Src & (1 << 4))
					{
						l_Temp++;
					}
				}
			}
			else
			{
				PASTA_ASSERT_MSG(false, "Invalid UF8 Char starts with b10 and not b0X or b11 ");
			}
			return l_Temp;
		}
		else
		{
			return l_Temp;
		}

	}

	uint32_t UTF8Utils::StrLen(const UTF8Char * _Src)
	{
		PASTA_ASSERT(_Src);
		uint32_t l_Local = 0;
		uint32_t l_Total = 0;
		while (*_Src)
		{
			l_Local = CountCharWidth(_Src[0]);
			l_Total += 1;
			_Src += l_Local;
		}
		return l_Total;
	}

	uint32_t UTF8Utils::GetLastBits(uint32_t _Code, uint32_t _Nth)
	{
		uint32_t l_Mask = ((1 << (_Nth)) - 1);
		return (_Code & l_Mask);
	};

	const UTF8Char * UTF8Utils::Advance(const UTF8Char * _Src, uint32_t & _Code)
	{
		PASTA_ASSERT(_Src);
		if (*_Src == 0)
		{
			_Code = 0;
			return _Src;
		}
		const uint32_t l_CurChar = *_Src;
		if (l_CurChar & (1 << 7))
		{
			if ((l_CurChar & (1 << 6))
				&& !(l_CurChar & (1 << 5)))
			{
				_Code = (GetLastBits(*_Src, 5) << 6)
					| (GetLastBits(*(_Src + 1), 6));
				return _Src + 2;
			}
			else
			{

				if ((l_CurChar & (1 << 5))
					&& !(l_CurChar & (1 << 4)))
				{
					_Code = (GetLastBits(*_Src, 4) << 12)
						| (GetLastBits(*(_Src + 1), 6) << 6)
						| (GetLastBits(*(_Src + 2), 6));
					return _Src + 3;
				}
				else
				{
					if ((l_CurChar & (1 << 4))
						&& !(l_CurChar & (1 << 3)))
					{
						_Code = (GetLastBits(*_Src, 3) << 18)
							| (GetLastBits(*(_Src + 1), 6) << 12)
							| (GetLastBits(*(_Src + 2), 6) << 6)
							| (GetLastBits(*(_Src + 3), 6));

						return _Src + 4;
					}
					else
					{
						//("Badly formed UTF-8 String");
						_Code = 0;
						return NULL;
					}
				}
			}
		}
		else
		{
			_Code = GetLastBits(*_Src, 7);
			return _Src + 1;
		}
	}

	//codepage is not modified
	wchar_t * UTF8Utils::utf8ToWchar(wchar_t * _Dest, const UTF8Char * _Src, uint32_t _MaxDestSize)
	{
		memset(_Dest,0, _MaxDestSize * sizeof(wchar_t));
		if (!_Src)
		{
			return _Dest;
		}

		uint32_t l_Index = 0;
		uint32_t l_DstChar = 0;
		while (*_Src)
		{
			if (l_Index > _MaxDestSize)
			{
				break;
			}
			_Src = Advance(_Src, l_DstChar);

			if (l_DstChar)
			{
				PASTA_ASSERT(l_DstChar <= 0xffffff);
				_Dest[l_Index++] = (wchar_t)l_DstChar;
			}
		}
		_Dest[_MaxDestSize - 1] = 0;
		return _Dest;
	};



}