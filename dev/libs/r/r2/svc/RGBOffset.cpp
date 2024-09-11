#include "stdafx.h"

#include "RGBOffset.hpp"

using namespace std;
r2::svc::RGBOffset::RGBOffset(){
	rgbOfs.resize(6);
}

void r2::svc::RGBOffset::bmpOp(r2::Bitmap & bmp)
{
	bmp.mkUber();
	bmp.shaderFlags |= USF_RGBOffset;
	bmp.updateShaderParam("uRGBOffset", rgbOfs.data(), 6);
}

r2::Tile* r2::svc::RGBOffset::make(r::Texture* tex, const float* rgbOfs, r2::TexFilter filter) {
	if (!tex) return nullptr;

	bool proceed = false;
	for (int i = 0; i < 6; ++i) 
		if (rgbOfs[i] != 0.0)
			proceed = true;

	if (!proceed) {
		dummy.setPos(0, 0);
		dummy.setSize(tex->getLogicalWidth(), tex->getLogicalHeight());
		dummy.setTexture(tex);

		rd.isFresh = true;
		return &dummy;
	}

	for (int i = 0; i < 6; ++i)
		if ((i & 1) == 0)
			this->rgbOfs[i] = rgbOfs[i] / tex->getWidth();
		else
			this->rgbOfs[i] = rgbOfs[i] / tex->getHeight();

	ops.clear();
	ops.push_back([=](r2::Bitmap& bmp) { bmpOp(bmp); });
	tex->setWrapModeUVW(PASTA_TEXTURE_MIRROR);
	return r2::svc::Capture::make(tex, ops, filter);
}
