#include "stdafx.h"

#include "Checksum.hpp"
#include "../tb64/turbob64.h"

using namespace r;

static bool	s_Initialized = false;
static u32 s_CRCTable[256];

void Checksum::Initialize(){
	if (!s_Initialized)
	{
		u32 i, j;
		u32 l_CRC;

		for (i = 0; i < 256; i++)
		{
			l_CRC = i << 24;
			for (j = 0; j < 8; j++)
			{
				if (l_CRC & 0x80000000)
					l_CRC = (l_CRC << 1) ^ 0x04c11db7;
				else
					l_CRC = l_CRC << 1;
			}
			s_CRCTable[i] = l_CRC;
		}

		s_Initialized = true;
	}
}

u32	Checksum::CRC32(const void *_Buffer, u32 _Size){
	if (!_Size || !_Buffer)
		return 0;

	if (!s_Initialized)
	{
		Checksum::Initialize();
	}

	u32 l_CRC;
	u8* l_Buffer = (u8*)_Buffer;

	u32 i;

	if (_Size >= 4)
	{
		// Optimized algorithm using table
		l_CRC = *l_Buffer++ << 24;
		l_CRC |= *l_Buffer++ << 16;
		l_CRC |= *l_Buffer++ << 8;
		l_CRC |= *l_Buffer++;
		l_CRC = ~ l_CRC;
		_Size -=4;

		for (i=0; i<_Size; i++)
		{
			l_CRC = (l_CRC << 8 | *l_Buffer++) ^ s_CRCTable[l_CRC >> 24];
		}
	}
	else
	{
		// Plain bit by bit algorithm
		u32 j;
		u8 l_Val;

		l_CRC = 0xFFFFFFFF;
		for (i=0; i<_Size; i++)
		{
			l_Val = *(l_Buffer++);
			for (j=0; j<8; j++)
			{
				if ((l_Val >> 7) ^ (l_CRC >> 31))
				{
					l_CRC = (l_CRC << 1) ^ 0x04c11db7;
				}
				else
				{
					l_CRC = (l_CRC << 1);
				}
				l_Val <<= 1;
			}
		}
	}

	return ~l_CRC;
}


////////////////////////////////////////////////
// Adler32 algorithm from zlib 1.2.3
////////////////////////////////////////////////

#define ADLER_BASE 65521UL    /* largest prime smaller than 65536 */
#define ADLER_NMAX 5552 /* ADLER_NMAX is the largest n such that 255n(n+1)/2 + (n+1)(ADLER_BASE-1) <= 2^32-1 */
#define ADLER_DO1(buf,i)  {adler += (buf)[i]; sum2 += adler;}
#define ADLER_DO2(buf,i)  ADLER_DO1(buf,i); ADLER_DO1(buf,i+1);
#define ADLER_DO4(buf,i)  ADLER_DO2(buf,i); ADLER_DO2(buf,i+2);
#define ADLER_DO8(buf,i)  ADLER_DO4(buf,i); ADLER_DO4(buf,i+4);
#define ADLER_DO16(buf)   ADLER_DO8(buf,0); ADLER_DO8(buf,8);

u32	Checksum::Adler32(const void *_Buffer, u32 _Size){
	if (!_Size || !_Buffer)
		return 0;
	u8* l_Buffer = (u8*)_Buffer;
	
	u32 adler;
	u32 sum2;
	u32 n;

	/* split Adler-32 into component sums */
	sum2 = 0;
	adler = 1;

	/* in case user likes doing a byte at a time, keep it fast */
	if (_Size == 1) {
		adler += l_Buffer[0];
		if (adler >= ADLER_BASE)
			adler -= ADLER_BASE;
		sum2 += adler;
		if (sum2 >= ADLER_BASE)
			sum2 -= ADLER_BASE;
		return adler | (sum2 << 16);
	}

	/* in case short lengths are provided, keep it somewhat fast */
	if (_Size < 16) {
		while (_Size--) {
			adler += *l_Buffer++;
			sum2 += adler;
		}
		if (adler >= ADLER_BASE)
			adler -= ADLER_BASE;
		sum2 %= ADLER_BASE;             /* only added so many ADLER_BASE's */
		return adler | (sum2 << 16);
	}

	/* do length ADLER_NMAX blocks -- requires just one modulo operation */
	while (_Size >= ADLER_NMAX) {
		_Size -= ADLER_NMAX;
		n = ADLER_NMAX / 16;          /* ADLER_NMAX is divisible by 16 */
		do {
			ADLER_DO16(l_Buffer);          /* 16 sums unrolled */
			l_Buffer += 16;
		} while (--n);
		adler %= ADLER_BASE;
		sum2 %= ADLER_BASE;
	}

	/* do remaining bytes (less than ADLER_NMAX, still just one modulo) */
	if (_Size) {                  /* avoid modulos if none remaining */
		while (_Size >= 16) {
			_Size -= 16;
			ADLER_DO16(l_Buffer);
			l_Buffer += 16;
		}
		while (_Size--) {
			adler += *l_Buffer++;
			sum2 += adler;
		}
		adler %= ADLER_BASE;
		sum2 %= ADLER_BASE;
	}

	/* return recombined sums */
	return adler | (sum2 << 16);
}

std::vector<r::u8> rs::Encoding::encodeBase64Vec(const r::u8* bytes, u32 size) {
	size_t encodedSize = tb64enclen(size);
	std::vector<r::u8> res;
	res.resize(encodedSize);
	size_t totalLen = tb64enc(bytes, size, res.data());
	return res;
}

std::string rs::Encoding::encodeBase64Str(const r::u8* bytes, u32 size) {
	size_t encodedSize = tb64enclen(size);
	std::string res;
	res.resize(encodedSize);
	size_t totalLen = tb64enc(bytes, size, (u8*)res.data());
	return res;
}

void rs::Encoding::decodeBase64Vec(std::vector<char>& _out, const std::string& _in) {
	_out.resize(_in.size());
	size_t sz = tb64dec((const u8*)_in.data(), _in.size(), (u8*)_out.data());
}

void rs::Encoding::decodeBase64Vec(std::vector<r::u8>& _out, const std::string& _in) {
	_out.resize(_in.size());
	size_t sz = tb64dec((const u8*) _in.data(), _in.size(), (u8*) _out.data());
}

void rs::Encoding::decodeBase64Buff(r::u8 * _out, int maxSize, const std::string& _in){
	int szEval = std::min<int>(maxSize,_in.size());
	if(_in.size()>maxSize){
		trace("ahem?");
		return;
	}
	int sz = tb64dec((const u8*)_in.data(), _in.size(), _out);
}
