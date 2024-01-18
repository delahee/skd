#include "stdafx.h"
#include "Uber.hpp"
#include "1-time/Profiler.h"

using namespace std;
using namespace Pasta;

r2::svc::Uber::Uber() {
	pyramidShader.resize(3);

	for (int i = 0; i < 3; i++) {
		ShaderProgramDescription shDesc;
		ShaderVarNames& shNames = shDesc.getVarNames();
		ShaderDescription& vsDesc = shDesc.getShaderDesc(ShaderStage::Vertex);
		ShaderDescription& fsDesc = shDesc.getShaderDesc(ShaderStage::Fragment);

#if defined(PASTA_NX)
		vsDesc.setPath("shaders/switch/BloomPyramid_vs.glsl");
		fsDesc.setPath("shaders/switch/BloomPyramid_fs.glsl");
#else
		vsDesc.setPath("shaders/glsl/BloomPyramid_vs.glsl");
		fsDesc.setPath("shaders/glsl/BloomPyramid_fs.glsl");
#endif
		
		switch (i) {
		case 0: fsDesc.addDefine("PYRAMID_THRESHOLD"); break;
		case 1: fsDesc.addDefine("PYRAMID_DOWNSAMPLE"); break;
		case 2: fsDesc.addDefine("PYRAMID_UPSAMPLE"); break;
		}

		auto shader = ShaderProgH::useLoad(&shDesc);
		pyramidShader[i] = shader;
		ShaderProgH::weakUse(shader);
		ShaderProgH::weakLoad(shader);
		ShaderProgH::releaseUnuse(shader);
	}
}

void r2::svc::Uber::bmpOp(r2::Bitmap& bmp, r::Texture* pyramidResult) {
	Pasta::Graphic* gfx = Pasta::Graphic::getMainGraphic();
	bmp.mkUber();
	if (distortionAmount.w != 0 && distortionEnabled) {
		bmp.shaderFlags |= USF_Distortion;
		float theta = PASTA_DEG2RAD(min(160.0f, distortionAmount.w));
		float sigma = 2.0f * tan(theta * 0.5f);
		distortionAmount.x = distortionAmount.w >= 0 ? theta : 1.0f / theta;
		distortionAmount.y = sigma;
		bmp.updateShaderParam(("uDistortionAmount"), distortionAmount.ptr(), 4);
	}
	else bmp.shaderFlags &= ~USF_Distortion;

	if (chromaticAberrationSettings.x != 0 && chromaticEnabled) {
		bmp.shaderFlags |= USF_ChromaticAberration;
		bmp.updateShaderParam(("uChromaticAberrationAmount"), chromaticAberrationSettings.ptr(), 2);
	}
	else bmp.shaderFlags &= ~USF_ChromaticAberration;

	if (glitchAmount.x != 0 && glitchEnabled) {
		bmp.shaderFlags |= USF_Glitch;
		bmp.updateShaderParam(("uGlitchParams"), glitchAmount.ptr(), 4);
	}
	else bmp.shaderFlags &= ~USF_Glitch;

	if (bloomIntensity != 0 && bloomEnabled) {
		bmp.shaderFlags |= USF_BloomPyramid;
		bmp.additionnalTexture = pyramidResult;
		bmp.updateShaderParam("uBloomIntensity", bloomIntensity);
		bmp.updateShaderParam("uBloomColor", bloomColor.ptr(), 3);
	}
	else bmp.shaderFlags &= ~USF_BloomPyramid;

	if (vignetteAmount.x != 0 && vignetteEnabled) {
		bmp.shaderFlags |= USF_Vignette;
		bmp.updateShaderParam(("uVignetteAmount"), vignetteAmount.ptr(), 3);
		bmp.updateShaderParam(("uVignetteColor"), vignetteColor.ptr(), 3);
	}
	else bmp.shaderFlags &= ~USF_Vignette;

	if (colorMatrixEnabled) {
		bmp.updateShaderParam(("uColorMatrix"), colorMatrixCtrl.mat.ptr(), 4 * 4);
		bmp.shaderFlags |= USF_ColorMatrix;
	}
	else bmp.shaderFlags &= ~USF_ColorMatrix;

}

r2::SingleFbPage* r2::svc::Uber::computeBloomPyramid(r::Texture* tex) {
	PASTA_CPU_GPU_AUTO_MARKER("svc Uber compute pyramid");
	pyramidBuffer.resize(pyramidSize);
	int width = tex->getWidth() * bloomScale;
	int height = tex->getHeight() * bloomScale;
	for (int i = 0; i < pyramidSize; i++) {
		width = max(width / 2, 1);
		height = max(height / 2, 1);
		pyramidBuffer[i] = r2::SurfacePool::get()->allocSingleFb(width, height, r2::TexFilter::TF_LINEAR, false);
		pyramidBuffer[i]->getRt()->setWrapModeUVW(PASTA_TEXTURE_CLAMP);
	}

	Bitmap* b = Bitmap::fromTexture(tex);
	if(tmpSc == nullptr) tmpSc = new r2::Scene();
	for (int i = 0; i < pyramidSize; i++) {
		Pasta::Texture* tex = pyramidBuffer[i]->getRt();

		auto sh = pyramidShader[i == 0 ? 0 : 1];
		{
			Pasta::ShaderParam* p = sh->getParam("uInvTexResolution0");
			if (p) {
				Pasta::Vector2 ires(1.0f / tex->getHeight(), 1.0f / tex->getHeight());
				p->setValue(ires.ptr());
			}
		}
		if (i == 0) {
			{
				Pasta::ShaderParam* p = sh->getParam("uThreshold");
				if (p) {
					float knee = bloomThreshold * bloomKnee;
					Pasta::Vector4 ires(bloomThreshold, bloomThreshold - knee, knee * 2.0f, 0.25f / knee);
					p->setValue(ires.ptr());
				}
			}
		}

		b->overrideShader = sh;
		b->texFiltering = r2::TexFilter::TF_LINEAR;
		b->setSize(tex->getWidth(), tex->getHeight());
		b->drawTo(tex, pyramidBuffer[i]->buffer, tmpSc);
		b->tile->set(tex);
	}

	Pasta::Texture* main = pyramidBuffer[pyramidSize - 1]->getRt();
	r2::SingleFbPage* pyramidResult = nullptr;
	for (int i = pyramidSize - 1; i >= 1; i--) {
		Pasta::Texture* bloom = pyramidBuffer[i - 1]->getRt();
		auto sh = pyramidShader[2];
		{
			Pasta::ShaderParam* p = sh->getParam("uInvTexResolution0");
			if (p) {
				Pasta::Vector2 ires(1.0f / main->getHeight(), 1.0f / main->getHeight());
				p->setValue(ires.ptr());
			}
		}
		b->tile->set(main);
		b->additionnalTexture = bloom;
		b->overrideShader = sh;
		b->texFiltering = r2::TexFilter::TF_LINEAR;

		if (pyramidResult) r2::SurfacePool::get()->freeSfb(pyramidResult);
		pyramidResult = r2::SurfacePool::get()->allocSingleFb(bloom->getWidth(), bloom->getHeight(), r2::TexFilter::TF_LINEAR, false);
		pyramidResult->getRt()->setWrapModeUVW(PASTA_TEXTURE_CLAMP);
		b->setSize(bloom->getWidth(), bloom->getHeight());
		b->drawTo(pyramidResult->getRt(), pyramidResult->buffer, tmpSc);
		main = pyramidResult->getRt();
	}
	Pools::bitmaps.free(b);

	for (int i = 0; i < pyramidSize; i++)
		r2::SurfacePool::get()->freeSfb(pyramidBuffer[i]);

	return pyramidResult;
}

r2::Tile* r2::svc::Uber::make(r::Texture* tex, r2::TexFilter filter) {
	if (!tex) return nullptr;
	PASTA_CPU_GPU_AUTO_MARKER("svc Uber make");
	Pasta::Graphic* gfx = Pasta::Graphic::getMainGraphic();

	r2::SingleFbPage* pyramidResult = nullptr;
	if (bloomIntensity != 0 && bloomEnabled)
		pyramidResult = computeBloomPyramid(tex);

	ops.clear();
	ops.push_back([this, pyramidResult](r2::Bitmap& bmp) { 
		r::Texture* pyramid = nullptr;
		if (pyramidResult != nullptr) pyramid = pyramidResult->getRt();
		bmpOp(bmp, pyramid);
	});
	tex->setWrapModeUVW(PASTA_TEXTURE_MIRROR);
	auto res =  r2::svc::Capture::make(tex, ops, filter);

	if(pyramidResult != nullptr) r2::SurfacePool::get()->freeSfb(pyramidResult);

	return res;
}

void r2::svc::Uber::blit(rs::GfxContext* gfx, const Pasta::Matrix44& trs, r2::Tile* tile) {
	if (!tile) return;

	Pasta::Texture * tex = tile->getTexture();
	if (!tex) return;
	tex->setWrapModeUVW(PASTA_TEXTURE_CLAMP);

	r2::SingleFbPage* pyramidResult = nullptr;
	r::Texture* pyramid = nullptr;
	if (bloomIntensity != 0 && bloomEnabled)
		pyramidResult = computeBloomPyramid(tex);
	if (pyramidResult != nullptr)
		pyramid = pyramidResult->getRt();

	Pasta::Graphic * g = Pasta::Graphic::getMainGraphic();
	Pasta::GraphicContext * ctx = Pasta::GraphicContext::GetCurrent();

	gfx->push();

	g->setDepthState(r2::GpuObjects::depthNoSupport);
	g->setTransparencyType(r::TransparencyType::TT_ALPHA);
	g->applyContextTransparency();
	g->setVertexDeclaration(Pasta::VD_POSITIONS | Pasta::VD_TEXCOORDS);
	g->setTexture(Pasta::ShaderStage::Fragment, 0, tex);
	g->setTexture(Pasta::ShaderStage::Fragment, 1, pyramid);
	g->applyContextTransparency();

	uint32_t shaderFlags = 0;

	shaderFlags |= Pasta::Graphic::BasicShaderFlags::BSF_TEXTURE;
	if (tex->isPremultipliedAlpha())
		shaderFlags |= Pasta::Graphic::BasicShaderFlags::BSF_PREMUL_ALPHA;

	Vars shaderValues;
		if ((distortionAmount.w != 0 || downloadTransitionAmount != 0) && distortionEnabled) {
			shaderFlags |= USF_Distortion;
			float theta = PASTA_DEG2RAD(min(160.0f, distortionAmount.w));
			float sigma = 2.0f * tan(theta * 0.5f);
			distortionAmount.x = distortionAmount.w >= 0 ? theta : 1.0f / theta;
			distortionAmount.y = sigma;
			shaderValues.set("uDistortionAmount", distortionAmount.ptr(), 4);
			shaderValues.set("uDownloadTransitionAmount", downloadTransitionAmount);
		}

		if (chromaticAberrationSettings.x != 0 && chromaticEnabled) {
			shaderFlags |= USF_ChromaticAberration;
			shaderValues.set("uChromaticAberrationAmount", chromaticAberrationSettings.ptr(), 2);
		}

		if (glitchAmount.x != 0 && glitchEnabled) {
			shaderFlags |= USF_Glitch;
			shaderValues.set("uGlitchParams", glitchAmount.ptr(), 4);
		}

		if (bloomIntensity != 0 && bloomEnabled) {
			shaderFlags |= USF_BloomPyramid;
			shaderValues.set("uBloomIntensity", bloomIntensity);
			shaderValues.set("uBloomColor", bloomColor.ptr(), 3);
		}

		if (vignetteAmount.x != 0 && vignetteEnabled) {
			shaderFlags |= USF_Vignette;
			shaderValues.set("uVignetteAmount", vignetteAmount.ptr(), 3);
			shaderValues.set("uVignetteColor", vignetteColor.ptr(), 3);
		}

		if (colorMatrixEnabled) {
			shaderFlags |= USF_ColorMatrix;
			shaderValues.set("uColorMatrix", colorMatrixCtrl.mat.ptr(), 4 * 4);
		}

	auto sh = r2::Lib::getShader(r2::Shader::SH_Uber, shaderFlags);
	r2::Lib::applyShaderValues(g, sh, shaderValues);
	g->setShader(sh);

	//snatch the good view matrix
	gfx->loadModelMatrix(trs);
	gfx->loadViewMatrix(gfx->viewMatrix);
	gfx->loadProjMatrix(gfx->projMatrix);

	const int stride = 3 + 2;
	float vertexData[4 * stride];//pos uv
	memset(vertexData, 0, 4 * stride * sizeof(float));
	uint32_t texCoordOffset = 3;//after pos

	{
		float *pos0 = &vertexData[0 * stride];
		float *pos1 = &vertexData[1 * stride];
		float *pos2 = &vertexData[2 * stride];
		float *pos3 = &vertexData[3 * stride];

		float v0x = tile->dx;
		float v1x = tile->dx + tile->width;

		float v0y = tile->dy;
		float v1y = tile->dy + tile->height;

		pos0[0] = v0x;
		pos0[1] = v0y;
		pos0[2] = 0;

		pos1[0] = v1x;
		pos1[1] = v0y;
		pos1[2] = 0;

		pos2[0] = v0x;
		pos2[1] = v1y;
		pos2[2] = 0;

		pos3[0] = v1x;
		pos3[1] = v1y;
		pos3[2] = 0;
	}

	// texcoords
	{
		float *tex0 = &vertexData[0 * stride + texCoordOffset];
		float *tex1 = &vertexData[1 * stride + texCoordOffset];
		float *tex2 = &vertexData[2 * stride + texCoordOffset];
		float *tex3 = &vertexData[3 * stride + texCoordOffset];

		tex0[0] = tile->u1; tex0[1] = tile->v1;
		tex1[0] = tile->u2; tex1[1] = tile->v1;
		tex2[0] = tile->u1; tex2[1] = tile->v2;
		tex3[0] = tile->u2; tex3[1] = tile->v2;
	}

	g->drawPrimitives(Pasta::PT_TRIANGLE_STRIP, 2, vertexData);

	g->setVertexDeclaration(0);
	g->setTexture(Pasta::ShaderStage::Fragment, 0, NULL);
	g->setTexture(Pasta::ShaderStage::Fragment, 1, NULL);

	gfx->pop();

	if (pyramidResult != nullptr) r2::SurfacePool::get()->freeSfb(pyramidResult);
}

void r2::svc::Uber::im() {
	
}

void r2::svc::Uber::upload(){
}
