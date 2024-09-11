#include "stdafx.h"

#include "Sprite3D.hpp"
#include "r2/GpuObjects.hpp"

using namespace r2;
using namespace r3;



Sprite3D::Sprite3D(Node* parent) : Super(parent) {}

Sprite3D::~Sprite3D() {}

void Sprite3D::dispose() {
	Super::dispose();
	shaderValues.dispose();
	alpha = 1.0f;
}

void Sprite3D::draw(rs::GfxContext* _g) {
	bool cont = drawPrepare(_g);
	if (!cont) return;
	 
	drawSubmitGeometry(_g);
	drawCleanup(_g);
}

void Sprite3D::reset() {
	Super::reset();
	shaderValues.dispose();
	depthRead = false;
	depthWrite = false;
	depthOnly = false;
	hasTexcoords = false;
	hasVertexColors = false;
	hasNormals = false;
	color = r::Color();
	blendmode = r::TransparencyType::TT_ALPHA;
	overrideShader = nullptr;
	texFiltering = r2::TexFilter::TF_INHERIT;
	alpha = 1.0f;
}

void Sprite3D::applyDepth(rs::GfxContext* ctx) {
	Pasta::Graphic* g = ctx->gfx;

	if (ctx->currentPass == rs::Pass::Basic
		|| ctx->currentPass == rs::Pass::EarlyDepth
		|| ctx->currentPass == rs::Pass::Picking) {
		Pasta::DepthStateID did = GpuObjects::depthNoSupport;
		if (depthRead && depthWrite)
			did = GpuObjects::depthReadWrite;
		else if (depthRead)
			did = GpuObjects::depthReadOnly;
		else if (depthWrite)
			did = GpuObjects::depthWriteOnly;
		g->setDepthState(did);
	}
	else if (ctx->currentPass == rs::Pass::DepthEqWrite) {
		g->setDepthState(GpuObjects::depthReadEq);
	}
}

void Sprite3D::applyBlendmode(rs::GfxContext* ctx) {
	Pasta::Graphic* g = ctx->gfx;
	Pasta::GraphicContext* gfxCtx = Pasta::GraphicContext::GetCurrent();
	if (depthOnly) {
		g->setBlendState(r2::GpuObjects::opaqueNoColor);
		return;
	}

	switch (ctx->currentPass) {
	case rs::Pass::EarlyDepth:
		gfxCtx->setBlendState(r2::GpuObjects::opaqueNoColor);
		return;
	case rs::Pass::Picking:
		g->setTransparencyType(TransparencyType::TT_OPAQUE);
		break;
	default: // basic or deptheq
		g->setTransparencyType(blendmode);
		break;
	}
	g->applyContextTransparency();
}

void Sprite3D::bindShader(rs::GfxContext * _g) {
	Pasta::Graphic* g = _g->gfx;
	Pasta::ShaderProgram * sh = overrideShader ? overrideShader : pickShader(_g);
	if (!sh) return;
	{
		Pasta::ShaderParam* p = sh->getParam("uModel");
		if (p) p->setValue(mat.transpose(), Pasta::ArrayLayout::AL_COLUMN_MAJOR);
	}

	r2::Lib::applyShaderValues(_g, sh, shaderValues);
	g->setShader(sh);
}

Pasta::ShaderProgram* Sprite3D::pickShader(rs::GfxContext* gfxCtx) {
	Pasta::Graphic* g = Pasta::Graphic::getMainGraphic();
	Pasta::GraphicContext* ctx = Pasta::GraphicContext::GetCurrent();

	Pasta::u32 flags = 0;
	flags |= Pasta::Graphic::BasicShaderFlags::BSF_UNIFORM_COLOR;
	if (hasTexcoords) {
		flags |= Pasta::Graphic::BasicShaderFlags::BSF_TEXTURE;
		flags |= Pasta::Graphic::BasicShaderFlags::BSF_KILL_ALPHA;
	}
	if (hasVertexColors) 
		flags |= Pasta::Graphic::BasicShaderFlags::BSF_VERTEX_COLOR;
	Pasta::Color gCol = g->getColor();
	float gAlpha = g->getAlpha();

	if (gfxCtx->currentPass == Pass::EarlyDepth)
		gCol.r = gCol.g = gCol.b = 0.0;

	Pasta::ShaderProgram* shader = nullptr;
	shader = r2::Lib::getShader(Shader::SH_Basic, flags);
	if (!shader)
		printf("no basic shader with such flags %d", flags);

	if (flags & Pasta::Graphic::BasicShaderFlags::BSF_UNIFORM_COLOR) {
		Pasta::ShaderParam* param = shader->getParam("uColor");
		PASTA_ASSERT(param);
		if (param) {
			Pasta::Color c = gCol;
			c.a = gAlpha;

			param->setValue(c.ptr());
		}
	}

	return shader;
}


bool Sprite3D::drawPrepare(rs::GfxContext* _g) {
	if (!shouldRenderThisPass(_g)) 
		return false;

	Pasta::Graphic* g = _g->gfx;
	Pasta::GraphicContext* ctx = Pasta::GraphicContext::GetCurrent();
	g->pushContext();

	applyDepth(_g);
	applyBlendmode(_g);

	if (hasVertexColors)
		int here = 0;

	g->setVertexDeclaration(Pasta::VD_POSITIONS 
		| (hasTexcoords ? Pasta::VD_TEXCOORDS : 0) 
		| (hasNormals ? Pasta::VD_NORMALS : 0) 
		| (hasVertexColors ? Pasta::VD_COLORS : 0));

	r::Color col = color;
	g->setColor(Pasta::Color(col.r, col.g, col.b, 1.0));
	g->setAlpha(col.a * _g->alpha.back());

	//set matrix
	if (!(nodeFlags & NF_MANUAL_MATRIX))
		syncMatrix(); //ensure matrix is correct
	_g->loadModelMatrix(mat);

	g->setTexture(Pasta::ShaderStage::Fragment, 0, r2::GpuObjects::whiteTex);

	bindShader(_g);

	return true;
}

void Sprite3D::drawCleanup(rs::GfxContext* _g) {
	Pasta::Graphic* g = _g->gfx;
	Pasta::GraphicContext* ctx = Pasta::GraphicContext::GetCurrent();
	g->setVertexDeclaration(0);
	g->setTexture(Pasta::ShaderStage::Fragment, 0, NULL);

	g->popContext();
}

bool Sprite3D::shouldRenderThisPass(rs::GfxContext* ctx) {
	if (!visible) return false;

	if (ctx->currentPass == rs::Pass::Picking) {
		if (nodeFlags & (NF_UTILITY | NF_EDITOR_FROZEN | NF_EDITOR_PROTECT))
			return false; // do not pick frozen, protected or custom shaded node
		return true;
	}

	bool earlyDepthCompatible = ctx->supportsEarlyDepth;
	earlyDepthCompatible &= depthWrite;
	earlyDepthCompatible &= blendmode == Pasta::TT_CLIP_ALPHA || blendmode == Pasta::TT_OPAQUE;
	earlyDepthCompatible &= !filter;

	switch (ctx->currentPass) {
	case rs::Pass::EarlyDepth: // fallthrough
	case rs::Pass::DepthEqWrite:
		return earlyDepthCompatible;
	case rs::Pass::Basic:
		return !earlyDepthCompatible;
	default:
		//PASTA_ASSERT_MSG(false, "invalid pass");
		return false;
	}
}

void Sprite3D::im(){
	using namespace ImGui;
	Super::im();

	if (!rd::Bits::is(nodeFlags, NF_EDITOR_HIDE_RENDERING) && ImGui::CollapsingHeader(ICON_MD_PALETTE " Sprite 3D")) {
		ImGui::Indent(); ImGui::PushItemWidth(124);

		ImGui::Checkbox("depthRead", &depthRead);
		ImGui::Checkbox("depthWrite", &depthWrite);
		ImGui::Checkbox("depthOnly", &depthOnly);

		ImGui::DragDouble("Alpha", &alpha, 1.0 / 255.0, 0.0, 2.0);
		ImGui::ColorPicker4("Color", color.ptr(), NULL);
		ImGui::Combo("blendmode", (int*)(&blendmode), r2::Im::blends, r::TransparencyType::TRANSPARENCY_TYPE_COUNT);

		ImGui::PopItemWidth(); ImGui::Unindent();
	}

	if (!rd::Bits::is(nodeFlags, NF_EDITOR_HIDE_RENDERING) && ImGui::CollapsingHeader(ICON_MD_AUTO_FIX_HIGH " Shader")) {
		ImGui::Indent();

		if (overrideShader) {
			ImGui::TextWrapped("This Sprite uses a custom shader");
		} else {
			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1, 0, 0, 1));
			ImGui::TextWrapped(ICON_MD_WARNING " Default shader editing on Sprite 3D is not currently supported");
			ImGui::PopStyleColor();
		}

		if (ImGui::TreeNode("Shader values")) {
			if (shaderValues.head)
				r2::Im::metadata(shaderValues.head);
			else {
				ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1, 0, 0, 1));
				ImGui::TextWrapped(ICON_MD_WARNING " No shader values");
				ImGui::PopStyleColor();
				if (Button("Create"))
					shaderValues.head = rd::Anon::fromPool();
			}
			ImGui::TreePop();
		}
		ImGui::Unindent();
	}
}

void Sprite3D::blendAlpha() {
	blendmode = Pasta::TT_ALPHA;
}

void Sprite3D::blendAdd() {
	blendmode = Pasta::TT_ADD;
}

void Sprite3D::texFilterLinear() {
	texFiltering = TF_LINEAR;
}

void Sprite3D::texFilterNearest() {
	texFiltering = TF_NEAREST;
}


void Sprite3D::bindTexture(rs::GfxContext* ctx, Pasta::Texture* tex, int slot) {
	Pasta::Graphic* g = ctx->gfx;

	if (!tex) {
		g->setTexture(Pasta::ShaderStage::Fragment, slot, nullptr);
		return;
	}

	if (texFiltering == r2::TexFilter::TF_INHERIT) {
		g->setTexture(Pasta::ShaderStage::Fragment, slot, tex);
		return;
	}

	Pasta::TextureFlags fl = tex->getFlags() & (PASTA_TEXTURE_MIN__MASK__ | PASTA_TEXTURE_MAG__MASK__ | PASTA_TEXTURE_MIP__MASK__);
	Pasta::TextureFlags ofl = fl;

	switch (texFiltering)
	{
	case r2::TexFilter::TF_LINEAR:
		fl |= PASTA_TEXTURE_LINEAR;
		tex->setFilterMode(PASTA_TEXTURE_LINEAR);
		break;
	case r2::TexFilter::TF_ANISO:
		fl |= PASTA_TEXTURE_ANISOTROPIC;
		tex->setFilterMode(PASTA_TEXTURE_ANISOTROPIC);
		break;
	default://defaults to nearest
		fl |= PASTA_TEXTURE_POINT;
		tex->setFilterMode(PASTA_TEXTURE_POINT);
		break;
	}

	g->setTexture(Pasta::ShaderStage::Fragment, slot, tex);
	tex->setFilterMode(ofl);
}


void Sprite3D::setColor(int col, float a) {
	color.r = ((col >> 16) & 0xff) / 255.0f;
	color.g = ((col >> 8) & 0xff) / 255.0f;
	color.b = ((col) & 0xff) / 255.0f;
	color.a = a;
}

void Sprite3D::setColor(const r::Color& col) {
	color = col;
}


double Sprite3D::getValue(TVar valType) {
	switch (valType) {
	case VX:
	case VY:
	case VScaleX:
	case VScaleY:
	case VRotation:
	case VScale:
	case VWidth:
	case VHeight:
		return Super::getValue(valType);

	case VR:		return color.r;
	case VG:		return color.g;
	case VB:		return color.b;
	case VA:		return color.a;
	case VAlpha:		return alpha;

	case VVisibility:	return (visible) ? 1.0 : 0.0;


	}
	return 0.0;
}

double Sprite3D::setValue(TVar valType, double val) {
	switch (valType) {
	case VX:
	case VY:
	case VScaleX:
	case VScaleY:
	case VRotation:
	case VScale:
	case VWidth:
	case VHeight:
		return Super::setValue(valType, val);

	case VR:		return color.r = val;
	case VG:		return color.g = val;
	case VB:		return color.b = val;
	case VA:		return color.a = val;

	case VAlpha:		 alpha = val;
		if (alpha < 0) alpha = 0;
		return alpha;
	case VVisibility:	return visible = (val >= 0.999) ? true : false;


	};
	return val;
}