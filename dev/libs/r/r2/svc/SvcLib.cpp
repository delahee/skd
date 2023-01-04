#include "stdafx.h"
#include "r2/svc/SvcLib.hpp"

void r2::svc::SvcLib::copyTexSubImage2D(eastl::vector<r::u32> &pixels, Pasta::TextureData* data, int px, int py, int w, int h) {
	if (!data)
		return;
	pixels.clear();
	pixels.reserve(w * h);
	int tw = 0;
	for (int ly = 0; ly < h; ++ly)
		for (int lx = 0; lx < w; ++lx)
		{
			Pasta::u8* tx = data->getRawTexel(px + lx, py + ly, tw);
			pixels.push_back( ((u32*)tx )[0] );
		}
}
