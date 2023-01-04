#include "stdafx.h"
#include "GpuObjects.hpp"
#include "1-Graphics/Graphic.h"
#include "1-Graphics/GraphicContext.h"
#include "1-Graphics/Texture.h"

using namespace r2;
using namespace Pasta;

Pasta::Texture *	GpuObjects::whiteTex	= nullptr;
const r2::Tile *	GpuObjects::whiteTile	= nullptr;
rd::Font *			GpuObjects::defaultFont	= nullptr;

bool GpuObjects::initialized = false;

Pasta::DepthStateID GpuObjects::depthNoSupport;
Pasta::DepthStateID GpuObjects::depthReadWrite;
Pasta::DepthStateID GpuObjects::depthReadOnly;
Pasta::DepthStateID GpuObjects::depthWriteOnly;
Pasta::DepthStateID GpuObjects::depthReadOnlyGreater;
Pasta::DepthStateID GpuObjects::depthReadEq;

Pasta::BlendStateID	GpuObjects::opaqueNoColor;
Pasta::BlendStateID	GpuObjects::alphaNoColor;
Pasta::BlendStateID	GpuObjects::alphaPremulNoColor;

//Pasta::CoverageStateID GpuObjects::noCoverage;
//Pasta::CoverageStateID GpuObjects::defaultCoverage;

void GpuObjects::init() {
	if (initialized) return;
	initialized = true;

	auto ctx = Pasta::GraphicContext::GetCurrent();
	auto loader = Pasta::TextureLoader::getSingleton();

	int tex[16];
	memset(tex, 0xff, sizeof(tex));
	whiteTex = ctx->CreateTexture(loader->loadRaw(tex, 4, 4));
	whiteTile = new Tile(whiteTex);

	//DepthState dn;
	//dn.m_write = false;
	//dn.m_func = Pasta::CompareFunc::Always;
	//depthNoSupport = ctx->CreateDepthStateID(dn);
	//does not seem to work, let's just put invalid
	depthNoSupport = PASTA_STATE_ID_INVALID;

	DepthState dro;
	dro.m_write = false;
	dro.m_func = Pasta::CompareFunc::LessEqual;
	depthReadOnly = ctx->CreateDepthStateID(dro);

	DepthState drw;
	drw.m_write = true;
	drw.m_func = Pasta::CompareFunc::LessEqual;
	depthReadWrite = ctx->CreateDepthStateID(drw);

	DepthState dwo;
	dwo.m_write = true;
	dwo.m_func = Pasta::CompareFunc::Always;
	depthWriteOnly = ctx->CreateDepthStateID(dwo);

	DepthState dre;
	dre.m_write = false;
	dre.m_func = Pasta::CompareFunc::Equal;
	depthReadEq = ctx->CreateDepthStateID(dre);

	DepthState drwg;
	drwg.m_write = false;
	drwg.m_func = Pasta::CompareFunc::Greater;
	depthReadOnlyGreater = ctx->CreateDepthStateID(drwg);

	{
		BlendState bs;
		bs = ctx->GetBlendState(ctx->GetBlendStateID_Alpha());
		bs.m_rtSlot[0].m_alphaWrite = false;
		bs.m_rtSlot[0].m_redWrite = false;
		bs.m_rtSlot[0].m_greenWrite = false;
		bs.m_rtSlot[0].m_blueWrite = false;
		alphaNoColor = ctx->CreateBlendStateID(bs);
	}

	{
		BlendState bs;
		bs = ctx->GetBlendState(ctx->GetBlendStateID_AlphaPremul());
		bs.m_rtSlot[0].m_alphaWrite = false;
		bs.m_rtSlot[0].m_redWrite = false;
		bs.m_rtSlot[0].m_greenWrite = false;
		bs.m_rtSlot[0].m_blueWrite = false;
		alphaPremulNoColor = ctx->CreateBlendStateID(bs);
	}

	{
		BlendState bs;
		bs.m_rtSlot[0].m_alphaWrite = false;
		bs.m_rtSlot[0].m_redWrite = false;
		bs.m_rtSlot[0].m_greenWrite = false;
		bs.m_rtSlot[0].m_blueWrite = false;
		opaqueNoColor = ctx->CreateBlendStateID(bs);
	}
	

	{
	//	CoverageState n;
	//	n.m_enabled = false;
	//	noCoverage = ctx->CreateCoverageStateID(n);
	}

	{
	//	CoverageState d;
	//	d.m_enabled = true;
	//	defaultCoverage = ctx->CreateCoverageStateID(d);
	}

	
}

void r2::GpuObjects::dispose()
{
	auto ctx = Pasta::GraphicContext::GetCurrent();

	ctx->DestroyTexture(whiteTex);
	whiteTex = nullptr;

	initialized = false;
}

void GpuObjects::onContextLost()
{

}

void GpuObjects::onContextRestore()
{
}
