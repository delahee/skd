#include "stdafx.h"
#include "Kernel.hpp"
#include "1-graphics/FrameBuffer.h"
#include "1-graphics/Graphic.h"
#include "1-time/Profiler.h"
#include "1-graphics/FrameBuffer.h"
#include "../Lib.hpp"

using namespace std;
using namespace Pasta;
using namespace r2;
using namespace r2::svc;

static Kernel * ker = nullptr;

static std::string uKernel  = "uKernel";
static std::string uSampleOffsetsXY = "uSampleOffsetsXY";

Kernel::Kernel() {

}

r2::svc::Kernel::~Kernel() {
	if (nodeCapture) {
		delete(nodeCapture);
		nodeCapture = nullptr;
	}
}

Kernel & Kernel::get() {
	if (ker == nullptr) 
		ker = new Kernel();
	return *ker;
}

r2::Tile* Kernel::makeBlur1D(Pasta::Texture* src, double size, double scale, double resolutionDivider, r2::TexFilter filter, bool horizontalOrVertical) {
	stub.mapTexture(src);
	return makeBlur1D(&stub, size, scale, resolutionDivider, filter, horizontalOrVertical);
}

r2::Tile* r2::svc::Kernel::makeBlur1D(r2::Node* src, double size, double scale, double resolutionDivider, r2::TexFilter filter, bool horizontalOrVertical) {
	//make a capture of adequate size
	double m = Kernel::getKernelWidth(size);
	double maxForPadI = ceil(m);

	//paddedCapture->rd.sc->clearColor = Color(0.1, 1, 1, 0.33);
	if (!nodeCapture) nodeCapture = new Capture();

	r2::Tile * tCapture = nodeCapture->make(src, filter, maxForPadI);
	r2::Tile * tBlur = makeBlur1D(tCapture, size, scale, resolutionDivider, filter, horizontalOrVertical);
	tBlur->translateCenterDiscrete(-maxForPadI,-maxForPadI);
	return tBlur;
}

r2::Tile * r2::svc::Kernel::makeBlur1D(r2::Tile * src, double size, double scale, double resolutionDivider, r2::TexFilter filter, bool horizontalOrVertical) {
	if (size <= 0) {
		dummy.copy(*src);
		rd.isFresh = true;
		return &dummy;
	}

	if (size >= 82)
		size = 82;
	int kernel_size = getKernelWidth(size);
	kernel.reserve(kernel_size);
	offsets.reserve(kernel_size * 2);

	Texture * tex = src->getTexture();
	int srcWidth = src->width;
	int srcHeight = src->height;

	int dstWidth = srcWidth;
	int dstHeight = srcHeight;

    if (horizontalOrVertical)
        dstWidth /= resolutionDivider;
    else
        dstHeight /= resolutionDivider;

	dstWidth = srcWidth + src->dx;
	dstHeight = srcHeight + src->dy;

	scale *= resolutionDivider;

	Lib::m_gaussian_kernel(kernel.data(), kernel_size, size);

	float * ofs = offsets.data();
	memset(ofs, 0, sizeof(float) * kernel_size * 2);

	for (int i = 0; i < kernel_size; i++) {
		if (!horizontalOrVertical) //thank you branch predictor
			ofs[i * 2 + 1] = scale * (i - (int)(kernel_size * 0.5)) / tex->getHeight();
		else
			ofs[i * 2] = scale * (i - (int)(kernel_size * 0.5)) / tex->getWidth();
	}
	tex->setWrapModeUVW(PASTA_TEXTURE_CLAMP);

	rd.filter = filter;
	rd.update(dstWidth, dstHeight);

	Texture *		res = rd.getWorkingTexture();
	FrameBuffer*	resFb = rd.getWorkingFB();
	r2::Tile*		dest = rd.getWorkingTile();
	Scene *			sc = rd.sc;
	Bitmap *		b = rd.bmp;

	res->setWrapModeUVW(PASTA_TEXTURE_CLAMP);

	b->resetTRS();
	b->x = 0;
	b->y = 0;

	b->tile->copy(*src);
	b->texFiltering = filter;
	if (horizontalOrVertical)
		b->scaleX = 1.0f / resolutionDivider;
	else
		b->scaleY = 1.0f / resolutionDivider;

    dest->resetTargetFlip();
    dest->setPos(0, 0);
    dest->setSize(dstWidth, dstHeight);
    dest->textureFlipY();

	b->mkUber();
	b->shaderFlags |= USF_Gaussian_Blur;
	b->updateShaderParam(uKernel, kernel.data(), kernel_size);
	b->updateShaderParam(uSampleOffsetsXY, offsets.data(), kernel_size * 2);	
	b->drawTo(res, resFb, sc);

	return rd.getDrawingTile();
}

r2::Tile* r2::svc::Kernel::makeBlur1D_KeepOffset(r2::Tile* src, double size, double scale, r2::TexFilter filter, bool horizontalOrVertical) {
	if (size <= 0) {
		dummy.copy(*src);
		rd.isFresh = true;
		return &dummy;
	}

	if (size >= 82)
		size = 82;
	int kernel_size = getKernelWidth(size);
	kernel.reserve(kernel_size);
	offsets.reserve(kernel_size * 2);

	Texture* tex = src->getTexture();
	int srcWidth = src->width;
	int srcHeight = src->height;

	int dstWidth = srcWidth;
	int dstHeight = srcHeight;

	dstWidth = srcWidth;
	dstHeight = srcHeight;

	Lib::m_gaussian_kernel(kernel.data(), kernel_size, size);

	float* ofs = offsets.data();
	memset(ofs, 0, sizeof(float) * kernel_size * 2);

	for (int i = 0; i < kernel_size; i++) {
		if (!horizontalOrVertical) //thank you branch predictor
			ofs[i * 2 + 1] = scale * (i - kernel_size * 0.5) / tex->getHeight();
		else
			ofs[i * 2] = scale * (i - kernel_size * 0.5) / tex->getWidth();
	}
	tex->setWrapModeUVW(PASTA_TEXTURE_CLAMP);

	rd.filter = filter;
	rd.update(dstWidth, dstHeight);
	
	Texture *		res = rd.getWorkingTexture();
	FrameBuffer*	resFb = rd.getWorkingFB();
	r2::Tile*		dest = rd.getWorkingTile();
	Scene *			sc = rd.sc;
	Bitmap *		b = rd.bmp;

	res->setWrapModeUVW(PASTA_TEXTURE_CLAMP);

	b->resetTRS();
	b->x = 0;
	b->y = 0;

	b->tile->copy(*src);
	b->tile->setCenterDiscrete(0, 0);

    dest->resetTargetFlip();
    dest->translateCenterDiscrete(src->dx, src->dy);
    dest->setPos(0, 0);
    dest->setSize(dstWidth, dstHeight);
    dest->textureFlipY();

	b->mkUber();
	b->shaderFlags |= USF_Gaussian_Blur;
	b->updateShaderParam(uKernel, kernel.data(), kernel_size);
	b->updateShaderParam(uSampleOffsetsXY, offsets.data(), kernel_size * 2);
	b->drawTo(res, resFb, sc);

	return rd.getDrawingTile();
}

void r2::svc::Kernel::im()
{
	if (nodeCapture)
		nodeCapture->im();

	rd.im();
}

