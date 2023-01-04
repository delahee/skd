#include "stdafx.h"
#include "Helper.hpp"

using namespace std;
using namespace r;
using namespace r2;

void r2::makeRGBOffset(r2::Sprite * spr, Pasta::Vector2 r, Pasta::Vector2 g, Pasta::Vector2 b) {
	spr->mkUber();
	Texture * tex = spr->getRenderTile()->getTexture();
	int width = tex->getWidth();
	int height = tex->getHeight();
	tex->setWrapModeUVW(PASTA_TEXTURE_CLAMP);
	spr->shaderFlags |= USF_RGBOffset;
	float rgb[6] = {
		r.x / width,
		r.y / height,

		g.x / width,
		g.y / height,

		b.x / width,
		b.y / height,
	};
	spr->setShaderParam(std::string("uRGBOffset"), rgb, 6);
}

void r2::makeBlurX(r2::Sprite * spr, float dx, float offsetScale) {
	spr->mkUber();
	Texture * tex = spr->getRenderTile()->getTexture();
	int width = tex->getWidth();
	int height = tex->getHeight();
	tex->setWrapModeUVW(PASTA_TEXTURE_CLAMP);
	spr->shaderFlags |= USF_Gaussian_Blur;

	int kernelx_size = (int)(dx / 0.65f + 0.5f) * 2 + 1;
	float *	kernelx = nullptr;
	float * offsetx = nullptr;

	kernelx = (float *)malloc(kernelx_size * sizeof(float));
	offsetx = (float *)malloc(kernelx_size * sizeof(float) * 2);//pack x and y

	r2::Lib::m_gaussian_kernel(kernelx, kernelx_size, dx);
	for (int i = 0; i < kernelx_size; i++) {
		offsetx[i * 2] = offsetScale * (i - kernelx_size * 0.5) / width;
		offsetx[i * 2 + 1] = 0.0;
	}
	spr->setShaderParam(string("uKernel"), kernelx, kernelx_size);
	spr->setShaderParam(string("uSampleOffsetsXY"), offsetx, kernelx_size * 2);

	free(kernelx);
	free(offsetx);
}

rd::Anon* r2::getRGBOffsetParam(r2::Sprite * spr ) {
	return spr->getShaderParam("uRGBOffset");
}