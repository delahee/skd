#include "stdafx.h"

#include "1-time/Profiler.h"
#include "r2/Scene.hpp"
#include "r2/OffscreenScene.hpp"
#include "PixelScene.hpp"

using namespace r2;
using namespace rd;
using namespace std;

PixelScene::PixelScene(int w,int h) : Super(){
	doClear = true;
	clearColor = r::Color(0.0f, 0.0f, 0.0f, 1.0f);

	if (w == -1) w = Scene::windowWidth();
	if (h == -1) h = Scene::windowHeight();

	sceneWidth = wantedWidth = w;
	sceneHeight = wantedHeight = h;

	areDimensionsOverriden = true;
	renderToBackBuffer = true;

	neutral();
	al.push_back(&tw);
	name = std::string("DtScene#") + to_string(uid);
}

void PixelScene::onResize(const Vector2& ns) {
	Super::onResize(ns);

	{
		sceneWidth = wantedWidth = ns.x;
		sceneHeight = wantedHeight = ns.y;
	}
}

void PixelScene::im() {
	using namespace ImGui;
	
	Super::im();

	if (ImGui::CollapsingHeader(ICON_MD_CORPORATE_FARE " Dt Scene")) {
		Indent(); 
		ImGui::Checkbox("Pixel Perfect", &pixelPerfect);
		ImGui::Checkbox("High Res Bloom", &highResBloom);
		ImGui::Checkbox("HDR", &HDR);
		ImGui::Checkbox("Activate Early-Depth", &activateEarlyDepth);
		if (TreeNode("Debug framebuffer")) {
			rd.im();
			TreePop();
		}
		ImGui::Separator();
		ImGui::Text("Render stats:");
		Indent();
		gBasic.im();
		if (activateEarlyDepth) {
			ImGui::Separator();
			gEarly.im();
			ImGui::Separator();
			gDepthEq.im();
		}
		Unindent();
		Unindent();
	}

	uber.im();
}

void PixelScene::testSettings() {

}

void PixelScene::neutral() {
	uber.bloomIntensity = 0.0f;
	uber.distortionAmount.w = 0.9f;
	setValue((rs::TVar)VCHROMA_SAMPLES, 3);

}


PixelScene::~PixelScene() {
	dispose();
}

void PixelScene::render(Pasta::Graphic* g) {
	if (skipRenderTarget) {
		Super::render(g);
		return;
	}

	for (RenderOp& ro : preRenderOps) ro(g);

	PASTA_CPU_GPU_AUTO_MARKER("Render DtScene");
	Pasta::GraphicContext* gfxCtx = Pasta::GraphicContext::GetCurrent();
	rs::GfxContext ctx(g);

	ctx.supportsEarlyDepth = activateEarlyDepth;

	float sX = pixelPerfect ? cameraScale.x : 1;
	float sY = pixelPerfect ? cameraScale.y : 1;

	rd.colorFormat = HDR ? Pasta::TextureFormat::RGBA16F : Pasta::TextureFormat::RGBA8;
	rd.update(wantedWidth / sX, wantedHeight / sY);

	FrameBuffer * _fb = rd.getWorkingFB();
	Texture * t = rd.getWorkingTexture();

	if (!_fb || !t) return;

	if (pixelPerfect) {
		cameraScale.x = 1;
		cameraScale.y = 1;
	}
	{
	PASTA_CPU_GPU_AUTO_MARKER("Rendering content");
	stdMatrix(&ctx, _fb->getWidth(), _fb->getHeight());
	ctx.push();

		ctx.pushTarget(_fb);

			for (RenderOpCtx& ro : preClearRenderOps) ro(&ctx);

			if (doClear) ctx.clear(PASTA_CLEAR_COLOR | PASTA_CLEAR_DEPTH, clearColor, ctx.zMax);

			for (RenderOpCtx& ro : postClearRenderOps) ro(&ctx);

			if (activateEarlyDepth) {
				//render all clipped
				if (!skipClip) {
					{
						PASTA_CPU_GPU_AUTO_MARKER("EarlyDepth pass");
						ctx.currentPass = rs::Pass::EarlyDepth;
						stdMatrix(&ctx, _fb->getWidth(), _fb->getHeight());
						drawRec(&ctx);
#ifdef _DEBUG
						gEarly = ctx;
						ctx.resetMetrics();
#endif
					}

					{
						PASTA_CPU_GPU_AUTO_MARKER("DepthEqWrite pass");
						ctx.currentPass = rs::Pass::DepthEqWrite;
						stdMatrix(&ctx, _fb->getWidth(), _fb->getHeight());
						drawRec(&ctx);
#ifdef _DEBUG
						gDepthEq = ctx;
						ctx.resetMetrics();
#endif
					}
				}
			}

			//render everything else ?
			if(!skipBasic)
			{
				PASTA_CPU_GPU_AUTO_MARKER("Basic pass");
				ctx.currentPass = rs::Pass::Basic;
				stdMatrix(&ctx, _fb->getWidth(), _fb->getHeight());
				drawRec(&ctx);
#ifdef _DEBUG
				gBasic = ctx;
				ctx.resetMetrics();
#endif
			}

			{
				PASTA_CPU_GPU_AUTO_MARKER("Extra pass");
				ctx.currentPass = rs::Pass::ExtraPass0;
				stdMatrix(&ctx, _fb->getWidth(), _fb->getHeight());
				drawRec(&ctx);
			}

			for (RenderOpCtx& ro : postRenderOps) ro(&ctx);

		ctx.popTarget();

	ctx.pop();
	}

	r2::Tile * dest = rd.getWorkingTile();

	//start post fxs
	/*r2::Tile* curTile = uber.make(dest->getTexture());

	if(curTile)
		finalComposite.copy(*curTile);*/

	if (renderToBackBuffer) {
		PASTA_CPU_GPU_AUTO_MARKER("Render to back buffer");
		ctx.push();
			syncMatrix();
			int w = -1;
			int h = -1;
			if (w == -1) w = windowWidth();
			if (h == -1) h = windowHeight();

			//std cam matrix was already done, here we just want to transfer image as it is...methinks
			ctx.loadViewMatrix(Matrix44::identity);
			ctx.loadModelMatrix(Matrix44::identity);
			ctx.loadProjMatrix(projMatrix = Matrix44::ortho(0, w / sX, h / sY, 0, ctx.zMin, ctx.zMax));
			//r2::Im::draw(&ctx, mat, &finalComposite);
			uber.bloomScale = highResBloom ? sX : 1.0f;
			uber.blit(&ctx, mat, dest);
		ctx.pop();
	}
	if (pixelPerfect) {
		cameraScale.x = sX;
		cameraScale.y = sY;
	}
}

rd::Tween* PixelScene::lastTween() {
	if (tw.tList.empty())
		return nullptr;
	return tw.tList.back();
}

void PixelScene::travellingWithDurMs(const Vector2i& to, float durMs) {
	if (isTravelling())
		cancelTravelling();

	auto screenCenter = Vector2(rs::Display::width() * 0.5, rs::Display::height() * 0.5);
	screenCenter.x /= cameraScale.x;
	screenCenter.y /= cameraScale.y;
	Vector2 dest = Vector2(to.x, to.y) - screenCenter;
	{
		double cur = getValue((rs::TVar)VCamPosX);
		double delta = to.x - cur;
		tw.create(this, (rs::TVar)VCamPosX, dest.x, TType::TEase, durMs);
	}
	{
		double cur = getValue((rs::TVar)VCamPosY);
		double delta = to.y - cur;
		tw.create(this, (rs::TVar)VCamPosY, dest.y, TType::TEase, durMs);
	}
}

void PixelScene::travellingWithSpeed(const Vector2i& to, float pixPerSec) {
	if(isTravelling())
		cancelTravelling();
	
	auto screenCenter = Vector2(rs::Display::width() * 0.5, rs::Display::height() * 0.5);
	screenCenter.x /= cameraScale.x;
	screenCenter.y /= cameraScale.y;
	Vector2 dest = Vector2(to.x,to.y) - screenCenter;
	Vector2 cur = Vector2(getValue((rs::TVar)VCamPosX), getValue((rs::TVar)VCamPosY));
	float time = (dest - cur).getNorm() / pixPerSec * 1000.0f; 
	{
		double cur = getValue((rs::TVar)VCamPosX);
		double delta = to.x - cur;
		tw.create(this, (rs::TVar)VCamPosX, dest.x, TType::TEase, time);
	}
	{
		double cur = getValue((rs::TVar)VCamPosY);
		double delta = to.y - cur;
		tw.create(this, (rs::TVar)VCamPosY, dest.y, TType::TEase, time);
	}
}

bool PixelScene::isTravelling() {
	return nullptr != tw.get(this, (rs::TVar)VCamPosX);
}

void PixelScene::cancelTravelling() {
	//beware we don't erase a tween with a locking mechanic here ?
	auto tx = tw.get(this, (rs::TVar)VCamPosX);
	auto ty = tw.get(this, (rs::TVar)VCamPosY);
	if (tx) {
		tx->onUpdate = {};
		if(tx->onEnd) tx->onEnd(this);
		tw.forceTerminateTween(tx);
	}
	if (ty) {
		ty->onUpdate = {};
		if (ty->onEnd) ty->onEnd(this);
		tw.forceTerminateTween(ty);
	}
}

double PixelScene::getValue(rs::TVar valType){
	switch (valType) {
	case VCMTX_HUE:				return uber.colorMatrixCtrl.hue;
	case VCMTX_SAT:				return uber.colorMatrixCtrl.sat;
	case VCMTX_VAL:				return uber.colorMatrixCtrl.val;

	case VDISTO_AMOUNT:			return uber.distortionAmount.w;
	case VDISTO_SCALE:			return uber.distortionAmount.z;
	case VDISTO_DL_TRANSITION:	return uber.downloadTransitionAmount;

	case VBLOOM_INTENSITY:		return uber.bloomIntensity;
	case VBLOOM_THRESH:			return uber.bloomThreshold;
	case VBLOOM_KNEE:			return uber.bloomKnee;
	case VBLOOM_PYRAMID_SIZE:	return uber.pyramidSize;
	case VBLOOM_CR:				return uber.bloomColor.r;
	case VBLOOM_CG:				return uber.bloomColor.g;
	case VBLOOM_CB:				return uber.bloomColor.b;
	case VBLOOM_CA:				return uber.bloomColor.a;

	case VGLITCH_AMOUNT:		return uber.glitchAmount.x;
	case VGLITCH_BIG:			return uber.glitchAmount.y;
	case VGLITCH_VERT:			return uber.glitchAmount.z;
	case VGLITCH_RES_DIV:		return uber.glitchAmount.w;

	case VVIGNETTE_INTENSITY:	return uber.vignetteAmount.x;
	case VVIGNETTE_SMOOTHNESS:	return uber.vignetteAmount.y;
	case VVIGNETTE_ROUNDNESS:	return uber.vignetteAmount.z;
	case VVIGNETTE_CR:			return uber.vignetteColor.r;
	case VVIGNETTE_CG:			return uber.vignetteColor.g;
	case VVIGNETTE_CB:			return uber.vignetteColor.b;
	case VVIGNETTE_CA:			return uber.vignetteColor.a;

	case VCHROMA_INTENSITY:		return uber.chromaticAberrationSettings.x;
	case VCHROMA_SAMPLES:		return uber.chromaticAberrationSettings.y;

	default: break;
	}
	return Super::getValue(valType);
}

double PixelScene::setValue(rs::TVar valType,double val) {
	switch (valType) {
	case VCMTX_HUE:				uber.colorMatrixCtrl.hue = val;			  uber.colorMatrixCtrl.pack(); break;
	case VCMTX_SAT:				uber.colorMatrixCtrl.sat = val;			  uber.colorMatrixCtrl.pack(); break;
	case VCMTX_VAL:				uber.colorMatrixCtrl.val = val;			  uber.colorMatrixCtrl.pack(); break;

	case VDISTO_AMOUNT:			uber.distortionAmount.w = val;			  break;
	case VDISTO_SCALE:			uber.distortionAmount.z = val;			  break;
	case VDISTO_DL_TRANSITION:	uber.downloadTransitionAmount = val;	  break;

	case VBLOOM_INTENSITY:		uber.bloomIntensity = val;				  break;
	case VBLOOM_THRESH:			uber.bloomThreshold = val;				  break;
	case VBLOOM_KNEE:			uber.bloomKnee = val;					  break;
	case VBLOOM_PYRAMID_SIZE:	uber.pyramidSize = val;					  break;
	case VBLOOM_CR:				uber.bloomColor.r = val;				  break;
	case VBLOOM_CG:				uber.bloomColor.g = val;				  break;
	case VBLOOM_CB:				uber.bloomColor.b = val;				  break;
	case VBLOOM_CA:				uber.bloomColor.a = val;				  break;

	case VGLITCH_AMOUNT:		uber.glitchAmount.x = val;				  break;
	case VGLITCH_BIG:			uber.glitchAmount.y = val;				  break;
	case VGLITCH_VERT:			uber.glitchAmount.z = val;				  break;
	case VGLITCH_RES_DIV:		uber.glitchAmount.w = val;				  break;

	case VVIGNETTE_INTENSITY:	uber.vignetteAmount.x = val;			  break;
	case VVIGNETTE_SMOOTHNESS:	uber.vignetteAmount.y = val;			  break;
	case VVIGNETTE_ROUNDNESS:	uber.vignetteAmount.z = val;			  break;
	case VVIGNETTE_CR:			uber.vignetteColor.r = val;				  break;
	case VVIGNETTE_CG:			uber.vignetteColor.g = val;				  break;
	case VVIGNETTE_CB:			uber.vignetteColor.b = val;				  break;
	case VVIGNETTE_CA:			uber.vignetteColor.a = val;				  break;

	case VCHROMA_INTENSITY:		uber.chromaticAberrationSettings.x = val; break;
	case VCHROMA_SAMPLES:		uber.chromaticAberrationSettings.y = val; break;

	default:					Super::setValue(valType, val);			  break;
	}
	return val;
}

void PixelScene::dispose() {
	Super::dispose();
}

rs::InputEvent PixelScene::transformEvent(rs::InputEvent& ev) {
	if(!pixelPerfect)
		return Super::transformEvent(ev);
	else {
		ev.relX /= cameraScale.x;
		ev.relY /= cameraScale.y;
		return Super::transformEvent(ev);
	}
}

void PixelScene::onEvent(rs::InputEvent& ev) {
	Super::onEvent(ev);
}

#undef SUPER