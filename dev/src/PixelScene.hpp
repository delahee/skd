#pragma once

#include "r2/OffscreenScene.hpp"
#include "rd/Anon.hpp"
#include "r2/Tile.hpp"

#include "1-graphics/Graphic.h"
#include "1-graphics/geo_vectors.h"

#include "r/Types.hpp"
#include "r2/RenderDoubleBuffer.hpp"

#include "r2/Lib.hpp"

#include "r2/svc/RGBOffset.hpp"
#include "r2/svc/Glitch.hpp"
#include "r2/svc/Bloom.hpp"
#include "r2/svc/ColorMatrix.hpp"
#include "r2/svc/Uber.hpp"


//this is a post processing volume actionnable scene
//this scene does not render int the backbuffer
class PixelScene : public r2::EarlyDepthScene {
public:
	typedef r2::EarlyDepthScene Super;
	enum DTS_TVAR {
		VCMTX_HUE = r2::Scene::VCamScaleZ + 1,
		VCMTX_SAT,
		VCMTX_VAL,

		VDISTO_AMOUNT,
		VDISTO_SCALE,
		VDISTO_DL_TRANSITION,

		VBLOOM_INTENSITY,
		VBLOOM_THRESH,
		VBLOOM_KNEE,
		VBLOOM_PYRAMID_SIZE,
		VBLOOM_CR,
		VBLOOM_CG,
		VBLOOM_CB,
		VBLOOM_CA,

		VGLITCH_AMOUNT,
		VGLITCH_BIG,
		VGLITCH_VERT,
		VGLITCH_RES_DIV,

		VVIGNETTE_INTENSITY,
		VVIGNETTE_SMOOTHNESS,
		VVIGNETTE_ROUNDNESS,
		VVIGNETTE_CR,
		VVIGNETTE_CG,
		VVIGNETTE_CB,
		VVIGNETTE_CA,

		VCHROMA_INTENSITY,
		VCHROMA_SAMPLES,
	};

									PixelScene(int width=-1, int height=-1);
	virtual							~PixelScene();

public:
	r2::RenderDoubleBuffer			rd;

	//
	r2::svc::Uber					uber;

	bool							skipRenderTarget = false;
	bool							pixelPerfect = true;
	bool							highResBloom = false;
	bool							HDR = true;
	bool							activateEarlyDepth = true;

	r2::TexFilter					colorMatrixFilter = r2::TexFilter::TF_NEAREST;
	bool							renderToBackBuffer = true;
	int								wantedWidth = 0;
	int								wantedHeight = 0;
	r2::Tile						finalComposite;

	rd::Tweener						tw;

public://public API
	rd::Tween*						lastTween();
	void							travellingWithDurMs(const Vector2i& to, float durMs);
	void							travellingWithSpeed(const Vector2i& to, float pixPerSec);
	bool							isTravelling();
	void							cancelTravelling();

	virtual double					getValue(rs::TVar valType) override;
	virtual double					setValue(rs::TVar valType, double val) override;

public://internal
	virtual void					dispose() override;
	virtual void					render(Pasta::Graphic *g) override;
	virtual void					onResize(const Vector2& ns) override;
	void							testSettings();
	void							neutral();
	virtual void					onEvent(rs::InputEvent& ev) override;
	virtual rs::InputEvent			transformEvent(rs::InputEvent& ev);
	
	virtual void					im()override;

	rs::GfxContext					gEarly;
	rs::GfxContext					gDepthEq;
	rs::GfxContext					gBasic;
};