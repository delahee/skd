#include "stdafx.h"

#include "Lib.hpp"

#include "rd/RscLib.hpp"
#include "GpuObjects.hpp"

#include "r2/Bitmap.hpp"
#include "r2/Scene.hpp"
#include "rs/GfxContext.hpp"

#include "r2/Helper.hpp"

#ifdef PASTA_OGL
#include "1-graphics/OGLGraphicContext.h"
#endif


using namespace r2;
using namespace rs;
using namespace Pasta;
using namespace std;

std::vector<TextureHolder> Lib::texCache;
std::vector<Texture*> Lib::renderTexCache;

Pasta::TextureData* r2::Lib::getTextureDataFromCache(const std::string& path){
	for(auto& c : texCache)
		if( c.path == path)
			return c.texData;
	return nullptr;
}

Pasta::Texture* Lib::getTexture(const std::string& path, r2::TexFilter filter, bool cacheTexture, bool cacheTextureData){
	Pasta::GraphicContext* ctx = Pasta::GraphicContext::GetCurrent();
	Pasta::TextureLoader* loader = Pasta::TextureLoader::getSingleton();

	if (cacheTextureData)
		cacheTexture = true;

	if (cacheTexture)
		for (TextureHolder th : texCache)
			if (th.tex->getResourceName() == path)
				return th.tex;

	Pasta::TextureData* texData = loader->load(path);
	if (!texData) {
		cerr << "[ERR]Cannot find texture " << path << endl;
		return 0;
	}
	u32 pfl = r2::toPastaTextureFilter(filter);
	Pasta::Texture* tex = ctx->CreateTexture(texData, pfl);

	double now = rs::Timer::stamp();
	rd::RscLib::makeBitArray(path.c_str(),texData);
	double then = rs::Timer::stamp();

	if(!cacheTextureData)
		loader->release(texData);
	if (cacheTexture) {
		TextureHolder th(path, tex);
		if (cacheTextureData) th.texData = texData;
		tex->setDebugName(th.path.c_str());
		texCache.push_back(std::move(th));
	}
	return tex;
}


Pasta::Texture* Lib::createTexture(Pasta::TextureData* texData, r2::TexFilter filter, bool tracked, const char * debugName){

	Pasta::GraphicContext* ctx = Pasta::GraphicContext::GetCurrent();
	Pasta::TextureLoader* loader = Pasta::TextureLoader::getSingleton();
	Pasta::Texture* tex = ctx->CreateTexture(texData, r2::toPastaTextureFilter(filter) );
	{
		TextureHolder th(debugName?debugName:"", tex);
		tex->setDebugName(debugName);
		if(tracked)
			texCache.push_back(std::move(th));
	}
	return tex;
}

Pasta::Texture* r2::Lib::createRenderTexture(int _width, int _height, r2::TexFilter filter, Pasta::TextureFormat::Enum textureFormat)
{
	Pasta::GraphicContext* ctx = Pasta::GraphicContext::GetCurrent();
	u32 fl = PASTA_TEXTURE_RENDER_TARGET | PASTA_TEXTURE_PREMUL_ALPHA;
	fl |= r2::toPastaTextureFilter(filter);
	Texture* t = ctx->CreateTexture(_width, _height, textureFormat, fl);
	renderTexCache.push_back(t);
	return t;
};

Pasta::Texture* r2::Lib::createDepthTexture(int _width, int _height, r2::TexFilter filter)
{
	Pasta::GraphicContext* ctx = Pasta::GraphicContext::GetCurrent();
	u32 fl = PASTA_TEXTURE_RENDER_TARGET ;
	fl |= r2::toPastaTextureFilter(filter);
	Texture* t = ctx->CreateTexture(_width, _height, Pasta::TextureFormat::FirstDepth,fl);
	renderTexCache.push_back(t);
	return t;
};

Pasta::FrameBuffer* r2::Lib::createRenderTextureFB(int _width, int _height, r2::TexFilter filter, Pasta::Texture* _in, bool destroyTextures, bool depth, Pasta::TextureFormat::Enum colorFormat)
{
	auto in = (_in) ? _in : createRenderTexture(_width, _height, filter, colorFormat);

	Pasta::FrameBuffer* fb;
	if (depth) {
		auto depth = createDepthTexture(_width, _height, filter);
		Pasta::Texture* tab[2] = {in, depth};
		fb = Pasta::GraphicContext::GetCurrent()->CreateFrameBuffer(tab, 2, destroyTextures);
	} else {
		fb = Pasta::GraphicContext::GetCurrent()->CreateFrameBuffer(&in, 1, destroyTextures);
	}
	
	return fb;
}

Pasta::FrameBuffer* r2::Lib::createRenderTextureFBDepth(int _width, int _height, r2::TexFilter filter, bool destroyTextures)
{
	auto in = createRenderTexture(_width, _height, filter);
	auto depth = createDepthTexture(_width, _height, filter);
	Pasta::Texture* tab[2] = {in, depth};
	return Pasta::GraphicContext::GetCurrent()->CreateFrameBuffer(tab, 2, destroyTextures);
}

void r2::Lib::destroyFramebuffer(Pasta::FrameBuffer* fb)
{
	Pasta::GraphicContext* ctx = Pasta::GraphicContext::GetCurrent();
	if (fb->getDepthAttachment())
	{
		Texture* tex = fb->getDepthAttachment()->m_texture;
		auto pos = find(renderTexCache.begin(), renderTexCache.end(), tex);
		if (pos != renderTexCache.end())
			renderTexCache.erase(pos);
	}
	if (fb->getColorAttachments())
	{
		Texture* tex = fb->getColorAttachments()->m_texture;
		auto pos = find(renderTexCache.begin(), renderTexCache.end(), tex);
		if (pos != renderTexCache.end())
			renderTexCache.erase(pos);
	}
	ctx->DestroyFrameBuffer(fb);
}

void Lib::destroyTexture(Pasta::Texture* tex)
{
	Pasta::GraphicContext* ctx = Pasta::GraphicContext::GetCurrent();

	for (int i = 0; i < texCache.size(); i++)
	{
		TextureHolder& h = texCache[i];
		if (h.tex == tex)
		{
			texCache.erase(texCache.begin() + i);
			break;
		}
	}

	if (tex->getFlags() & PASTA_TEXTURE_RENDER_TARGET)
	{
		auto pos = find(renderTexCache.begin(), renderTexCache.end(), tex);
		if (pos != renderTexCache.end())
			renderTexCache.erase(pos);
	}
	ctx->DestroyTexture(tex);
}

double r2::Lib::dt()
{
	return 0.0;
}

ShaderProgramDescription* msdfShaderProgDesc = nullptr;
ShaderProgramDescription* uberShaderProgDesc = nullptr;

std::unordered_map<Pasta::u32, Pasta::ShaderProgram*> r2::Lib::uberShaders;
std::unordered_map<Pasta::u32, Pasta::ShaderProgram*> r2::Lib::msdfShaders;

void r2::Lib::init()
{
	rs::Sys::init();
	GpuObjects::init();

	ShaderProgramDescription* basicShaderProgDesc = new ShaderProgramDescription();
	loadShader(basicShaderProgDesc, "Basic");
	for (int f = 0; f < 64; f++) // cache the basic shader
		cacheShader(basicShaderProgDesc, f, nullptr);
	uberShaderProgDesc = new ShaderProgramDescription();
	loadShader(uberShaderProgDesc, "Uber");
	msdfShaderProgDesc = new ShaderProgramDescription();
	loadShader(msdfShaderProgDesc, "MSDF");
}

void r2::Lib::dispose()
{
	GpuObjects::dispose();
	Graphic::releaseBasicShaders();
	releaseShader(&uberShaders);
	releaseShader(&msdfShaders);
}

void r2::Lib::m_gaussian_kernel(float* dest, int size, float radius)
{
	float* k;
	float rs, s2, sum;
	float sigma = 1.6f;
	float tetha = 2.25f;
	int r, hsize = size / 2;

	s2 = 1.0f / expf(sigma * sigma * tetha);
	rs = sigma / radius;

	k = dest;
	sum = 0.0f;

	/* compute gaussian kernel */
	for (r = -hsize; r <= hsize; r++)
	{
		float x = r * rs;
		float v = (1.0f / expf(x * x)) - s2;
		v = std::max(v, 0.0f);
		*k = v;
		sum += v;
		k++;
	}

	/* normalize */
	if (sum > 0.0f)
	{
		float isum = 1.0f / sum;
		for (r = 0; r < size; r++)
			dest[r] *= isum;
	}
}

Pasta::Texture* Lib::tex_Blur(Pasta::Texture* src, float dx, float dy, float offsetScale /*= 1.0*/, r2::TexFilter filter, Pasta::Texture* dst)
{
	if (((dx / 0.65f + 0.5f) * 2 + 1) > 256)
		dx = (((256 - 1) / 2) - 0.5f) * 0.65f;
	if (((dy / 0.65f + 0.5f) * 2 + 1) > 256)
		dy = (((256 - 1) / 2) - 0.5f) * 0.65f;

	// ensure we don't wrap
	src->setWrapModeUVW(PASTA_TEXTURE_CLAMP);
	int width = src->getWidth();
	int height = src->getHeight();

	Pasta::GraphicContext* ctx = Pasta::GraphicContext::GetCurrent();
	if (dx < FLT_EPSILON && dy < FLT_EPSILON)
	{
		Texture* final = (dst != nullptr) ? dst : r2::Lib::createRenderTexture(src->getWidth(), src->getHeight(), filter);
		final->setWrapModeUVW(PASTA_TEXTURE_CLAMP);
		tex_Copy(final, src);
		return final;
	}
	bool deleteSrc = false;
	Texture* originalSrc = src;

	Texture* res = nullptr;
	FrameBuffer* resFb = nullptr;

	Texture* tmp = nullptr;
	if (!dst)
		dst = r2::Lib::createRenderTexture(width, height, filter);

	if (dx > 0 && dy > 0)
		tmp = r2::Lib::createRenderTexture(width, height, filter);
	else
		tmp = dst;

	tmp->setWrapModeUVW(PASTA_TEXTURE_MIRROR);

	FrameBuffer* tmpFb = ctx->CreateFrameBuffer(&tmp, 1, false);

	if (dx > 0)
	{
		float* kernelx = nullptr;
		float* offsetx = nullptr;
		int kernelx_size = (int)(dx / 0.65f + 0.5f) * 2 + 1;

		kernelx = (float*)malloc(kernelx_size * sizeof(float));
		offsetx = (float*)malloc(kernelx_size * sizeof(float) * 2);  // pack x and y

		m_gaussian_kernel(kernelx, kernelx_size, dx);
		for (int i = 0; i < kernelx_size; i++)
		{
			offsetx[i * 2] = offsetScale * (i - kernelx_size * 0.5) / width;
			offsetx[i * 2 + 1] = 0.0;
		}

		Bitmap* b = Bitmap::fromTexture(src);
		b->mkUber();
		b->shaderFlags |= USF_Gaussian_Blur;
		b->setShaderParam(string("uKernel"), kernelx, kernelx_size);
		b->setShaderParam(string("uSampleOffsetsXY"), offsetx, kernelx_size * 2);
		b->drawTo(tmp, tmpFb);
		b->destroy();
		b = nullptr;
		free(kernelx);
		kernelx = nullptr;
		free(offsetx);
		offsetx = nullptr;

		if (dy <= 0)
		{  // no y, prepare to return
			// res = tmp;
			// resFb = ctx->CreateFrameBuffer(&tmp, 1, false);
			ctx->DestroyFrameBuffer(tmpFb);
			tmpFb = nullptr;
			return tmp;
		}
		else
		{  // dx>0 && dy >= 0

			res = dst;
			res->setWrapModeUVW(PASTA_TEXTURE_CLAMP);
			resFb = ctx->CreateFrameBuffer(&res, 1, false);
			src = tmp;
			deleteSrc = true;
			ctx->DestroyFrameBuffer(tmpFb);
			tmpFb = nullptr;
		}
	}
	else
	{					// no dx
		res = tmp;		// recycle data
		resFb = tmpFb;  // recycle data
		tmpFb = nullptr;
	}

	if (dy > 0)
	{
		int kernely_size = (int)(dy / 0.65f + 0.5f) * 2 + 1;
		float* kernely = nullptr;
		float* offsety = nullptr;

		kernely = (float*)malloc(kernely_size * sizeof(float));
		offsety = (float*)malloc(kernely_size * sizeof(float) * 2);  // pack x and y

		m_gaussian_kernel(kernely, kernely_size, dy);
		for (int i = 0; i < kernely_size; i++)
		{
			offsety[i * 2] = 0.0;
			offsety[i * 2 + 1] = offsetScale * (i - kernely_size * 0.5) / height;
		}

		Bitmap* b = Bitmap::fromTexture(src);
		b->mkUber();
		b->shaderFlags |= USF_Gaussian_Blur;
		b->setShaderParam(string("uKernel"), kernely, kernely_size);
		b->setShaderParam(string("uSampleOffsetsXY"), offsety, kernely_size * 2);
		b->drawTo(res, resFb);
		b->destroy();
		b = nullptr;
		free(kernely);
		free(offsety);
	}

	if (deleteSrc)
	{
		Lib::destroyTexture(src);
		src = nullptr;
	}

	if (resFb)
	{
		ctx->DestroyFrameBuffer(resFb);
		resFb = nullptr;
	}

	return res;
}

Pasta::Texture* r2::Lib::tex_ColorMatrix(Pasta::Texture* src, Pasta::Matrix44 mat, r2::TexFilter filter, Pasta::Texture* dst){
	Pasta::GraphicContext* ctx = Pasta::GraphicContext::GetCurrent();
	Pasta::Graphic* grap = Pasta::Graphic::getMainGraphic();

	int width = src->getWidth();
	int height = src->getHeight();
	Texture* res = (dst == nullptr) ? r2::Lib::createRenderTexture(width, height, filter) : dst;

	FrameBuffer* resFb = ctx->CreateFrameBuffer(&res, 1, false);

	Bitmap* bit = Bitmap::fromTexture(src);
	bit->mkUber();
	bit->shaderFlags |= USF_ColorMatrix;
	bit->setShaderParam(string("uColorMatrix"), mat.ptr(), 4 * 4);
	bit->drawTo(res, resFb);
	delete bit;

	ctx->DestroyFrameBuffer(resFb);

	return res;
}

Pasta::Texture* r2::Lib::tex_Bloom(const BloomCtrl& ctrl)
{
	Pasta::GraphicContext* ctx = Pasta::GraphicContext::GetCurrent();
	Pasta::Graphic* g = Pasta::Graphic::getMainGraphic();

	Pasta::Texture* texSrc = ctrl.src;
	texSrc->setWrapModeUVW(PASTA_TEXTURE_CLAMP);
	int width = texSrc->getWidth();
	int height = texSrc->getHeight();

	Bitmap* bHighPass = Bitmap::fromTexture(texSrc);
	bHighPass->mkUber();
	bHighPass->shaderFlags |= USF_Bloom;
	bHighPass->setShaderParam(string("uPass"), ctrl.pass);
	bHighPass->setShaderParam(string("uMul"), ctrl.mul.ptr(), 4);
	bHighPass->setShaderParam(string("uLuminanceVector"), ctrl.luminanceVector.ptr(), 3);
	Pasta::Texture* texHighPass = r2::Lib::createRenderTexture(width, height, r2::TexFilter::TF_NEAREST);
	FrameBuffer* highPassFb = ctx->CreateFrameBuffer(&texHighPass, 1, false);
	bHighPass->drawTo(texHighPass, highPassFb);

	Node* composite = new Node();

	Bitmap* bSrc = Bitmap::fromTexture(texSrc, composite);
	bSrc->visible = !ctrl.skipColorPass;

	Pasta::Texture* texBlur = tex_Blur(texHighPass, ctrl.dx, ctrl.dy, ctrl.offsetScale, ctrl.blurFilter);
	texBlur->setWrapModeUVW(PASTA_TEXTURE_CLAMP);
	Bitmap* bBlur = Bitmap::fromTexture(texBlur, composite);
	bBlur->blendmode = Pasta::TransparencyType::TT_ADD;
	bBlur->visible = !ctrl.skipBloomPass;

	Texture* res = (ctrl.dst) ? ctrl.dst : r2::Lib::createRenderTexture(width, height, ctrl.renderFilter);
	FrameBuffer* resFb = ctx->CreateFrameBuffer(&res, 1, false);
	composite->drawTo(res, resFb);

	ctx->DestroyFrameBuffer(resFb);
	ctx->DestroyFrameBuffer(highPassFb);
	r2::Lib::destroyTexture(texBlur);
	r2::Lib::destroyTexture(texHighPass);
	if(bHighPass) delete bHighPass;
	if(bSrc) delete bSrc;
	if(bBlur) delete bBlur;
	if(composite) delete composite;

	return res;
}

void r2::Lib::tex_Copy(Pasta::Texture* dest, Pasta::Texture* src)
{
	Pasta::GraphicContext* ctx = Pasta::GraphicContext::GetCurrent();
	Pasta::Graphic* g = Pasta::Graphic::getMainGraphic();
	rs::GfxContext _g = rs::GfxContext(g);

	Scene* s = new Scene();
	Bitmap* b = r2::Bitmap::fromTexture(src, s);

	s->stdMatrix(&_g, src->getWidth(), src->getHeight());

	_g.pushScissor();
	_g.setScissor(0, 0, src->getWidth(), src->getHeight());

	ctx->startFrame();

	FrameBuffer* destFb = ctx->CreateFrameBuffer(&dest, 1, false);
	s->drawInto(&_g, s, dest, destFb);
	ctx->DestroyFrameBuffer(destFb);

	_g.popScissor();

	b->destroy();
	delete s;
}

Pasta::Texture* r2::Lib::tex_Clone(Pasta::Texture* src)
{
	Texture* dst = r2::Lib::createRenderTexture(src->getWidth(), src->getHeight(), r2::TexFilter::TF_NEAREST);
	r2::Lib::tex_Copy(dst, src);
	return dst;
}

Pasta::Texture* r2::Lib::tex_RGBOffset(Pasta::Texture* src, Pasta::Vector2 dr, Pasta::Vector2 dg, Pasta::Vector2 db, r2::TexFilter filter, Pasta::Texture* dst)
{
	Pasta::GraphicContext* ctx = Pasta::GraphicContext::GetCurrent();
	Pasta::Graphic* g = Pasta::Graphic::getMainGraphic();

	int width = src->getWidth();
	int height = src->getHeight();

	src->setWrapModeUVW(PASTA_TEXTURE_MIRROR);

	Texture* res = dst == nullptr ? r2::Lib::createRenderTexture(width, height, filter) : dst;
	FrameBuffer* resFb = ctx->CreateFrameBuffer(&res, 1, false);

	Bitmap* b = Bitmap::fromTexture(src);
	b->mkUber();
	b->shaderFlags |= USF_RGBOffset;
	float rgb[6] = {
		-dr.x / width, dr.y / height,

		-dg.x / width, dg.y / height,

		-db.x / width, db.y / height,
	};

	b->setShaderParam(string("uRGBOffset"), rgb, 6);
	b->drawTo(res, resFb);
	delete b;

	ctx->DestroyFrameBuffer(resFb);

	return res;
}

Pasta::Texture* r2::Lib::tex_Glitch(Pasta::Texture* src,
									float intensityX,
									float percentBigX,
									float intensityY,
									float percentBigY,
									float phase,
									r2::TexFilter filter,
									Pasta::Texture* dst)
{
	Pasta::GraphicContext* ctx = Pasta::GraphicContext::GetCurrent();
	Pasta::Graphic* g = Pasta::Graphic::getMainGraphic();

	// src->setWrapModeUVW(PASTA_TEXTURE_CLAMP);

	int width = src->getWidth();
	int height = src->getHeight();
	Texture* res = (dst == nullptr) ? r2::Lib::createRenderTexture(width, height, filter) : dst;

	FrameBuffer* resFb = ctx->CreateFrameBuffer(&res, 1, false);

	Bitmap* b = Bitmap::fromTexture(src);
	b->mkUber();
	b->shaderFlags |= USF_Glitch;
	b->setShaderParam("uGlitchParams", Vector4(intensityX,intensityY, percentBigX, percentBigY).ptr(),4);
	//b->setShaderParam(string("iBigGlitchPercentX"), percentBigX);
	//b->setShaderParam(string("iGlitchIntensityY"), intensityY);
	//b->setShaderParam(string("iBigGlitchPercentY"), percentBigY);
	b->drawTo(res, resFb);
	delete b;

	ctx->DestroyFrameBuffer(resFb);

	return res;
}

static std::vector<std::string> mkVsDefs(Pasta::u32 flags)
{
	std::vector<std::string> res;
	if (flags & Graphic::BSF_VERTEX_COLOR)
		res.push_back("VERTEX_COLOR");
	if (flags & Graphic::BSF_UNIFORM_COLOR)
		res.push_back("UNIFORM_COLOR");
	if (flags & Graphic::BSF_TEXTURE)
		res.push_back("TEXTURE");
	if (flags & Graphic::BSF_PREMUL_ALPHA)
		res.push_back("PREMUL_ALPHA");
	if (flags & Graphic::BSF_KILL_ALPHA)
		res.push_back("KILL_ALPHA");
	if (flags & Graphic::BSF_TEXTURE_ALPHA_ONLY)
		res.push_back("TEXTURE_ALPHA_ONLY");

	bool hasBlur = flags & UberShaderFlags::USF_Gaussian_Blur;
	bool hasRGBOffset = flags & UberShaderFlags::USF_RGBOffset;
	bool hasGlitchPass = flags & UberShaderFlags::USF_Glitch;
	bool hasBloomPass = flags & UberShaderFlags::USF_Bloom;
	bool hasDisplace = flags & UberShaderFlags::USF_DisplacementMapping;
	bool hasColorMatrix = flags & UberShaderFlags::USF_ColorMatrix;
	bool hasFXAA = flags & UberShaderFlags::USF_FXAA;
	bool hasDistortion = flags & UberShaderFlags::USF_Distortion;
	bool hasVignette = flags & UberShaderFlags::USF_Vignette;
	bool hasBloomPyramid = flags & UberShaderFlags::USF_BloomPyramid;
	bool hasDithering = flags & UberShaderFlags::USF_Dither;

 	if (hasFXAA)
		res.push_back("HAS_FXAA");
	if (hasDithering)
		res.push_back("HAS_DITHERING");

		
	return res;
}

static std::vector<std::string> mkFsDefs(Pasta::u32 flags)
{
	std::vector<std::string> res;
	if (flags & Graphic::BSF_VERTEX_COLOR)
		res.push_back("VERTEX_COLOR");
	if (flags & Graphic::BSF_UNIFORM_COLOR)
		res.push_back("UNIFORM_COLOR");
	if (flags & Graphic::BSF_TEXTURE)
		res.push_back("TEXTURE");
	if (flags & Graphic::BSF_PREMUL_ALPHA)
		res.push_back("PREMUL_ALPHA");
	if (flags & Graphic::BSF_KILL_ALPHA)
		res.push_back("KILL_ALPHA");
	if (flags & Graphic::BSF_TEXTURE_ALPHA_ONLY)
		res.push_back("TEXTURE_ALPHA_ONLY");

	bool hasBlur = flags & UberShaderFlags::USF_Gaussian_Blur;
	bool hasBloomPass = flags & UberShaderFlags::USF_Bloom;
	bool hasRGBOffset = flags & UberShaderFlags::USF_RGBOffset;
	bool hasGlitchPass = flags & UberShaderFlags::USF_Glitch;
	bool hasDisplace = flags & UberShaderFlags::USF_DisplacementMapping;
	bool hasColorMatrix = flags & UberShaderFlags::USF_ColorMatrix;
	bool hasFXAA = flags & UberShaderFlags::USF_FXAA;
	bool hasDissolve = flags & UberShaderFlags::USF_Dissolve;
	bool dissolveUseTex = flags & UberShaderFlags::USF_DissolveUseTexture;
	bool hasDithering = flags & UberShaderFlags::USF_Dither;
	bool showNoise = flags & UberShaderFlags::USF_ShowNoise;
	bool hasDistortion = flags & UberShaderFlags::USF_Distortion;
	bool hasChromaticAberration = flags & UberShaderFlags::USF_ChromaticAberration;
	bool hasVignette = flags & UberShaderFlags::USF_Vignette;
	bool hasBloomPyramid = flags & UberShaderFlags::USF_BloomPyramid;
	bool hasColorAdd = flags & UberShaderFlags::USF_ColorAdd;
	bool hasRenderRes = flags & UberShaderFlags::USF_RenderResolution;


	if (hasBlur)
		res.push_back("HAS_GBLUR");
	if (hasBloomPass)
		res.push_back("HAS_BLOOM");
	if (hasRGBOffset)
		res.push_back("HAS_RGB_OFFSET");
	if (hasGlitchPass)
		res.push_back("HAS_GLITCH");
	if (hasDisplace)
		res.push_back("HAS_DISPLACE");
	if (hasColorMatrix)
		res.push_back("HAS_COLOR_MATRIX");
	if (hasColorAdd)
		res.push_back("HAS_COLOR_ADD");
	if (hasFXAA)
		res.push_back("HAS_FXAA");
	if (hasDissolve){
		res.push_back("HAS_DISSOLVE");
		if (dissolveUseTex)
			res.push_back("DISSOLVE_USE_TEXTURE");
		if (showNoise)
			res.push_back("RENDER_NOISE");
	}
	if (hasDithering)
		res.push_back("HAS_DITHERING");
	if (hasDistortion)
		res.push_back("HAS_DISTORTION");
	if (hasChromaticAberration)
		res.push_back("HAS_CHROMATIC_ABERRATION");
	if (hasBloomPyramid)
		res.push_back("HAS_BLOOM_PYRAMID");
	if (hasVignette)
		res.push_back("HAS_VIGNETTE");
	if (hasRenderRes)
		res.push_back("HAS_RENDER_RES");

	return res;
}

Pasta::GraphicContextSettings r2::Lib::getGraphicsSettings() {
#ifdef PASTA_OGL
	Pasta::OGLGraphicContext::setOpenGLVersion(4, 1); //Necessary for debugging with RenderDoc
#endif
	GraphicContextSettings settings;
	settings.m_numRWBufferSlots[ShaderStage::Vertex] = 1;
	settings.m_numRWBufferSlots[ShaderStage::Compute] = 1;
	settings.m_numTextureSlots[ShaderStage::Compute] = 1;
	return settings;
}

r::u64 r2::Lib::getTileHash(r2::Tile* t) {
	r::u64 id = 0;
	
	id |= (r::u64)t->x;
	id |= ((r::u64)t->y)<<16ull;
	id |= ((r::u64)t->width)<<24ull;
	id |= ((r::u64)t->height)<<8ull;

	if (t->getTexture())
		id ^= Checksum::CRC32(t->getTexture()->getPath());

	return id;
};

void r2::Lib::loadShader(ShaderProgramDescription* desc, std::string path) {
	ShaderProgramDescription& shaderProgDesc = *desc;
	ShaderDescription& vsDesc = shaderProgDesc.getShaderDesc(ShaderStage::Vertex);
	ShaderDescription& fsDesc = shaderProgDesc.getShaderDesc(ShaderStage::Fragment);

#if defined(PASTA_NX)
	vsDesc.setPath(("shaders/switch/" + path + "_vs.glsl").c_str());
	fsDesc.setPath(("shaders/switch/" + path + "_fs.glsl").c_str());
#else
	vsDesc.setPath(("shaders/glsl/" + path + "_vs.glsl").c_str());
	fsDesc.setPath(("shaders/glsl/" + path + "_fs.glsl").c_str());
#endif
}

void r2::Lib::releaseShader(std::unordered_map<Pasta::u32, Pasta::ShaderProgram*>* map) {
	for (auto it = map->begin(); it != map->end(); ++it) {
		ShaderProgH::weakRelease((*it).second);
		ShaderProgH::weakUnuse((*it).second);
	}
	map->clear();
}

Pasta::ShaderProgram* r2::Lib::getShader(r2::Shader shaderType, Pasta::u32 shaderFlags) {
	Pasta::Graphic* g = Pasta::Graphic::getMainGraphic();
	switch (shaderType) {
	case r2::Shader::SH_Basic:
		return g->getBasicShader(shaderFlags);
	case r2::Shader::SH_Uber:
		if (uberShaders[shaderFlags] == nullptr)
			cacheShader(uberShaderProgDesc, shaderFlags, &uberShaders);
		return uberShaders[shaderFlags];
	case r2::Shader::SH_MSDF:
		if (msdfShaders[shaderFlags] == nullptr)
			cacheShader(msdfShaderProgDesc, shaderFlags, &msdfShaders);
		return msdfShaders[shaderFlags];
	}
	return nullptr;
}

void r2::Lib::cacheShader(ShaderProgramDescription* shaderProgDesc, Pasta::u32 flags, std::unordered_map<Pasta::u32, Pasta::ShaderProgram*>* map) {
	auto vsDefs = mkVsDefs(flags);
	auto fsDefs = mkFsDefs(flags);
	ShaderDescription& vsDesc = shaderProgDesc->getShaderDesc(ShaderStage::Vertex);
	ShaderDescription& fsDesc = shaderProgDesc->getShaderDesc(ShaderStage::Fragment);

	vsDesc.clearDefines();
	for (auto d : vsDefs)
		vsDesc.addDefine(d.c_str());

	fsDesc.clearDefines();
	for (auto d : fsDefs)
		fsDesc.addDefine(d.c_str());

	ShaderProgram* shaderProg = ShaderProgH::use(shaderProgDesc);
	r2::Lib::setShader(flags, shaderProg, map);
	ShaderProgH::unuse(shaderProg);
}

void r2::Lib::setShader(Pasta::u32 shaderFlags, Pasta::ShaderProgram* shader, std::unordered_map<Pasta::u32, Pasta::ShaderProgram*>* map) {
	if (map == nullptr) {
		Graphic::setBasicShader(shaderFlags, shader);
		return;
	}

	ShaderProgH::weakUse(shader);
	ShaderProgH::weakLoad(shader);

	auto it = map->find(shaderFlags);
	if (it != map->end())
	{
		ShaderProgH::weakRelease((*it).second);
		ShaderProgH::weakUnuse((*it).second);
	}

	if (shader)
		(*map)[shaderFlags] = shader;
	else
		map->erase(shaderFlags);
}

Pasta::ShaderProgram * r2::Lib::loadCustomShader(const char * shaderName, std::vector<const char*> defines) {
	return loadCustomShader((shaderName + string("_vs.glsl")).c_str(), (shaderName + string("_fs.glsl")).c_str(), "140", defines);
}

Pasta::ShaderProgram* r2::Lib::loadCustomShader(const char* pathVS, const char* pathFS, const char* version, std::vector<const char*> defines) {
	ShaderProgramDescription shDesc;
	ShaderVarNames& shNames = shDesc.getVarNames();
	ShaderDescription& vsDesc = shDesc.getShaderDesc(ShaderStage::Vertex);
	ShaderDescription& fsDesc = shDesc.getShaderDesc(ShaderStage::Fragment);
#if defined(PASTA_NX)
	vsDesc.setPath((string("shaders/switch/") + pathVS).c_str());
	fsDesc.setPath((string("shaders/switch/") + pathFS).c_str());
#else
	vsDesc.setPath((string("shaders/glsl/") + pathVS).c_str());
	fsDesc.setPath((string("shaders/glsl/") + pathFS).c_str());
#endif

	vsDesc.setShaderVersion(version);
	fsDesc.setShaderVersion(version);

	for (int i = 0; i < defines.size(); i++) {
		vsDesc.addDefine(defines[i]);
		fsDesc.addDefine(defines[i]);
	}

	Pasta::ShaderProgram* shader = ShaderProgH::useLoad(&shDesc);
	ShaderProgH::weakUse(shader);
	ShaderProgH::weakLoad(shader);
	ShaderProgH::releaseUnuse(shader);
	return shader;
}

void r2::Lib::applyShaderValues(rs::GfxContext* gctx, Pasta::ShaderProgram* shader, rd::Vars & shaderValues) {
	Pasta::Graphic* g = gctx->gfx;
	{
		Pasta::ShaderParam* p = shader->getParam("uTime");
		float now = (float)rs::Timer::now;
		if (shaderValues.has("uTimeOffset"))
			now += shaderValues.getFloat("uTimeOffset");
		if (p) 
			p->setValue(now);
	}

	auto value = shaderValues.head;
	while (value) {
		Pasta::ShaderParam* p = shader->getParam(value->name);
		if (p) {
			switch (p->getType()) {
			case Pasta::ShaderParamType::SP_INT:			p->setValue(value->asInt()); break;
			case Pasta::ShaderParamType::SP_INT2:			// asIntBuffer
			case Pasta::ShaderParamType::SP_INT3:			// asIntBuffer
			case Pasta::ShaderParamType::SP_INT4:			p->setValue(value->asIntBuffer()); break;
			case Pasta::ShaderParamType::SP_INT_ARRAY:		p->setValueArray(value->asIntBuffer(), value->getSize()); break;
			case Pasta::ShaderParamType::SP_INT2_ARRAY:		p->setValueArray(value->asIntBuffer(), value->getSize() >> 1); break;
			case Pasta::ShaderParamType::SP_INT3_ARRAY:		p->setValueArray(value->asIntBuffer(), value->getSize() / 3); break;
			case Pasta::ShaderParamType::SP_INT4_ARRAY:		p->setValueArray(value->asIntBuffer(), value->getSize() >> 2); break;

			case Pasta::ShaderParamType::SP_FLOAT:			p->setValue(value->asFloat()); break;
			case Pasta::ShaderParamType::SP_FLOAT2:			// asFloatBuffer
			case Pasta::ShaderParamType::SP_FLOAT3:			// asFloatBuffer
			case Pasta::ShaderParamType::SP_FLOAT4:			p->setValue(value->asFloatBuffer()); break;
			case Pasta::ShaderParamType::SP_FLOAT_ARRAY:	p->setValueArray(value->asFloatBuffer(), value->getSize()); break;
			case Pasta::ShaderParamType::SP_FLOAT2_ARRAY:	p->setValueArray(value->asFloatBuffer(), value->getSize() >> 1); break;
			case Pasta::ShaderParamType::SP_FLOAT3_ARRAY:	p->setValueArray(value->asFloatBuffer(), value->getSize() / 3); break;
			case Pasta::ShaderParamType::SP_FLOAT4_ARRAY:	p->setValueArray(value->asFloatBuffer(), value->getSize() >> 2); break;

			case Pasta::ShaderParamType::SP_MATRIX44:
				p->setValue(Matrix44(value->asFloatBuffer()), Pasta::AL_COLUMN_MAJOR); break;

			default: assert(true);
			}
		}

		value = value->sibling;
	}

	//setup resolutions, todo strip allocations out of this
	for (int i = 0; i < 2; i++) {
		Texture* t = g->getTexture(Pasta::ShaderStage::Fragment, i);
		if (!t) continue;
		{
			Pasta::ShaderParam* p = shader->getParam("uTexResolution" + std::to_string(i));
			if (p) p->setValue(Pasta::Vector2(t->getWidth(), t->getHeight()).ptr());
		}

		{
			Pasta::ShaderParam* p = shader->getParam("uInvTexResolution" + std::to_string(i));
			if (p) p->setValue(Pasta::Vector2(1.0f / t->getWidth(), 1.0f / t->getHeight()).ptr());
		}
	}

	Pasta::ShaderParam* p = shader->getParam("uRenderRes");
	if (p) {
		Vector2 screenRes;
		Pasta::FrameBuffer*b = gctx->curBuffer;
		if (b)
			screenRes = { (float)b->getWidth(), (float)b->getHeight() };
		else
			screenRes = rs::Display::getSize();
		p->setValue(screenRes.ptr());
	}

}

r::Color r2::Lib::intToColor24(int rgb)
{
	return r::Color(((rgb >> 16) & 0xff) / 255.0f, ((rgb >> 8) & 0xff) / 255.0f, (rgb & 0xff) / 255.0f);
}

r::Color r2::Lib::getColor(int rgb, float alpha)
{
	return r::Color(((rgb >> 16) & 0xff) / 255.0f, ((rgb >> 8) & 0xff) / 255.0f, (rgb & 0xff) / 255.0f, alpha);
}

void r2::Lib::changeType(r2::Node* nu, r2::Node* old)
{
	r2::Node* parent = old->parent;
	int idx = parent ? parent->getChildIndex(old) : 0;
	eastl::vector<r2::Node*> oldChildren = old->children;
	old->removeAllChildren();
	old->clone(nu);
	for (r2::Node* child : oldChildren)
		nu->addChild(child);
	if(parent) parent->addChildAt(nu, idx);
}

void r2::Lib::imTex(const char* filter)
{
	if (ImGui::CollapsingHeader("Loaded Textures", ImGuiTreeNodeFlags_::ImGuiTreeNodeFlags_DefaultOpen))
	{
		ImGui::Text("Nb Tex: %lu", texCache.size());

		sort(texCache.begin(), texCache.end(), [](TextureHolder& t0, TextureHolder& t1) { return t0.path < t1.path; });

		for (auto& th : texCache)
		{
			if (strlen(filter) > 0)
			{
				auto pos = th.path.find(filter);
				if (pos == std::string::npos)
					continue;
			}

			ImGui::Separator();
			ImGui::Text(th.path.c_str());
			ShadedTexture* st = rd::Pools::allocForFrame();
			st->texture = th.tex;
			r2::Im::imTextureInfos(st->texture);
			ImGui::Image((ImTextureID)st, ImVec2(150, 150));
			
			if (ImGui::IsItemHovered())
			{
				ImGui::BeginTooltip();
				ImGui::Image((ImTextureID)st, ImVec2(512, 512));
				ImGui::EndTooltip();
			}
			ImGui::SameLine();
			ImGui::Text("Size ( kb ): %d", (th.tex->getWidth() * th.tex->getHeight() * 4) >> 10);
		}
	}

	if (ImGui::CollapsingHeader("Render Textures", 0))
	{
		ImGui::Text("Nb Tex: %lu", renderTexCache.size());

		sort(renderTexCache.begin(), renderTexCache.end(), [](Pasta::Texture* t0, Pasta::Texture* t1) { return uintptr_t(t0) < uintptr_t(t1); });

		int depthSkipped = 0;
		for (auto& th : renderTexCache)
			if (th->getFormat() >= Pasta::TextureFormat::FirstDepth)
				depthSkipped++;
		ImGui::Text("%d depth textures skipped", depthSkipped);

		for (auto& th : renderTexCache)
		{
			if (th->getFormat() >= Pasta::TextureFormat::FirstDepth)
				continue;

			const char* name = th->getDebugName();
			if (strlen(filter) > 0)
			{
				if (!name)
					continue;
				auto pos = std::string(name).find(filter);
				if (pos == std::string::npos)
					continue;
			}

			ImGui::Separator();
			if (name)
				ImGui::Text(name);
			if (th->hasPath())
				ImGui::Text(th->getPath());
			ShadedTexture* st = rd::Pools::allocForFrame();
			st->texture = th;
			r2::Im::imTextureInfos(st->texture);
			ImGui::Image((ImTextureID)st, ImVec2(150, 150), ImVec2(0, 1), ImVec2(1, 0));
			if (ImGui::IsItemHovered()) {
				ImGui::BeginTooltip();
				ImGui::Image((ImTextureID)st, ImVec2(512, 512), ImVec2(0, 1), ImVec2(1, 0));
				ImGui::EndTooltip();
			}
			ImGui::SameLine();
			ImGui::Text("Size ( kb ): %d", (th->getWidth() * th->getHeight() * 4) >> 10);
		}
	}
}

r::Vector2 r2::Lib::screenToGlobal(r2::Scene* sc, r::Vector2 pt){
	sc->stdMatrix(nullptr);
	return sc->viewMatrix.inverse() * pt;
}

string r2::Lib::loadContent(const char* path)
{
	Pasta::FileMgr* fmgr = Pasta::FileMgr::getSingleton();
	char* txt = (char*)fmgr->load(path);
	string res;
	if (txt){
		res = txt;
		fmgr->release(txt);
	}
	return res;
}

bool r2::Lib::saveContent(const char* path, const std::string& content)
{
	Pasta::FileMgr* fmgr = Pasta::FileMgr::getSingleton();
	Pasta::File* f = fmgr->createFile(path,"");
	bool done = f->open(Pasta::File::FileAttribute::FA_WRITE);
	if (!f || !done)
	{
		trace("Error cannot create file! " + string(path));
		return false;
	}
	f->writeRawText(content.c_str(), content.size());
	f->flush();
	f->close();
	return true;
}

void r2::GlitchControl::setVertical(bool onOff){
	glitchAmount.z = onOff ? 1.0 : 0.0;
}

bool r2::GlitchControl::im() {
	bool changed = false;
	changed |= ImGui::DragFloat("Amount", &glitchAmount.x, 0.1f);
	glitchAmount.y *= 100.0f;
	changed |= ImGui::DragFloat("Big glitch amount", &glitchAmount.y, 0.1f, 0.0f, 25.0f, "%0.1f%%");
	glitchAmount.y /= 100.0f;
	bool checked = (glitchAmount.z == 1);
	changed |= ImGui::Checkbox("Vertical", &checked);
	glitchAmount.z = checked ? 1 : 0;
	changed |= ImGui::DragFloat("Resolution divider", &glitchAmount.w, 0.1f);
	return changed;
}

void r2::GlitchControl::readFromShader(r2::Sprite* spr) {
	if (spr->shader != Shader::SH_Basic) {
		rd::Anon* value = spr->getShaderParam("uGlitchParams");
		if (value)
			glitchAmount = Vector4(spr->getShaderParam("uGlitchParams")->asFloatBuffer());
		else {
			neutral();
			upload(spr);
		}

	}
}

void r2::GlitchControl::upload(r2::Sprite* spr) {
	rd::Bits::toggle(spr->shaderFlags, USF_Glitch, enabled);
	spr->updateShaderParam("uGlitchParams", glitchAmount.ptr(), 4);
}

void r2::ColorMatrixControl::pack() {
	Matrix44 matSettings = Matrix44::identity;
	if(mode == ColorMatrixMode::HSV) {
		rd::ColorLib::colorHSV(matSettings, hue, sat, val);
		mat = (matSettings.transpose());
	}
	else if( mode == ColorMatrixMode::Colorize){
		rd::ColorLib::colorColorize(matSettings, tint, ratioNew, ratioOld);
		mat = (matSettings.transpose());
	}
	else {
		//you do you ;)
	}
}

void r2::ColorMatrixControl::setup(r2::Sprite* spr) {
	spr->mkUber();
	spr->shaderFlags |= UberShaderFlags::USF_ColorMatrix;
}

void r2::ColorMatrixControl::upload(r2::Sprite* spr) {
	spr->updateShaderParam("uColorMatrix", mat.ptr(), 16);
}

void r2::ColorMatrixControl::sync(r2::Sprite* spr){
	pack();
	setup(spr);
	upload(spr);
}

void r2::ColorMatrixControl::readFromShader(r2::Sprite* spr) {
	if (spr->shader != Shader::SH_Basic) {
		Matrix44 m;
		rd::Anon* param = spr->getShaderParam("uColorMatrix");
		if (param) memcpy(m.ptr(), param->asFloatBuffer(), 16*4);
		mat = m.transpose();
		mode = ColorMatrixMode::Matrix;
	}
}

std::string transparencytype_to_string(Pasta::TransparencyType tt) {
	switch (tt) {
	case Pasta::TT_OPAQUE: return "opaque";
	case Pasta::TT_CLIP_ALPHA: return "clip_alpha";
	case Pasta::TT_ALPHA: return "alpha";
	case Pasta::TT_ADD: return "add";
	case Pasta::TT_SCREEN: return "screen";
	case Pasta::TT_MULTIPLY: return "multiply";
	case Pasta::TT_ALPHA_MULTIPLY: return "alpha_multiply";
	case Pasta::TT_ERASE: return "erase";
	}
	return "unknown";
}

void r2::VignetteControl::neutral() {
	amount = r::Vector3(1, 1, 1);
	color = r::Color(0, 0, 0, 1);
}

void r2::VignetteControl::cut() {
	amount = r::Vector3(2, 0, 1);
	color = r::Color(0, 0, 0, 1);
}

bool r2::VignetteControl::im(){
	using namespace ImGui;
	bool chg = false;
	if (Button("neutral")) {
		neutral();
		chg = true;
	}
	SameLine();
	if (Button("cut")) {
		cut();
		chg = true;
	}
	
	chg |= ImGui::DragFloat("Intensity", &amount.x, 0.01f, 0, 3);
	chg |= ImGui::DragFloat("Smoothness", &amount.y, 0.01f, 0, 5);
	chg |= ImGui::DragFloat("Roundness", &amount.z, 0.01f, 6, 1);
	chg |= ColorEdit3("color", color.ptr());
	return chg;
}

void r2::VignetteControl::readFromShader(r2::Sprite* spr){
	if (spr->shader == Shader::SH_Basic) return;

	if (spr->hasShaderParam("uVignetteAmount")) {
		auto amnt = spr->getShaderParam("uVignetteAmount")->asFloatBuffer();
		if (amnt) amount = r::Vector3(amnt[0], amnt[1], amnt[2]);
	}

	if (spr->hasShaderParam("uVignetteColor")) {
		auto col = spr->getShaderParam("uVignetteColor")->asFloatBuffer();
		if (col) color.set(col[0], col[1], col[2]);
	}
}

void r2::VignetteControl::upload(r2::Sprite* spr){
	spr->updateShaderParam(("uVignetteAmount"), amount.ptr(), 3);
	spr->updateShaderParam(("uVignetteColor"),	color.ptr(), 3);
}

void r2::VignetteControl::setup(r2::Sprite* spr){
	spr->mkUber();
	spr->shaderFlags |= UberShaderFlags::USF_Vignette | UberShaderFlags::USF_RenderResolution;
}
