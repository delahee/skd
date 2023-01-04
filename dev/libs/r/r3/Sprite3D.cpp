#include "stdafx.h"

#include "Sprite3D.hpp"
#include "r2/GpuObjects.hpp"

using namespace r2;
using namespace r3;

#define SUPER Node3D

Sprite3D::Sprite3D(Node* parent) : SUPER(parent) {}

Sprite3D::~Sprite3D() {}

void Sprite3D::dispose() {
	SUPER::dispose();
	alpha = 1.0f;
}

void Sprite3D::draw(rs::GfxContext* _g) {
	bool cont = drawPrepare(_g);
	if (!cont) return;
	 
	drawSubmitGeometry(_g);
	drawCleanup(_g);
}

void Sprite3D::applyDepth(rs::GfxContext* _g) {
	Pasta::Graphic* g = _g->gfx;
	Pasta::GraphicContext* ctx = _g->getGpuContext();

	if (_g->currentPass == rs::Pass::Basic
	||  _g->currentPass == rs::Pass::EarlyDepth
	||  _g->currentPass == rs::Pass::Picking) {
		Pasta::DepthStateID did = GpuObjects::depthNoSupport;
		if (depthRead && depthWrite)
			did = GpuObjects::depthReadWrite;
		else if (depthRead)
			did = GpuObjects::depthReadOnly;
		else if (depthWrite)
			did = GpuObjects::depthWriteOnly;
		g->setDepthState(did);
	}
	else if (_g->currentPass == rs::Pass::DepthEqWrite) {
		g->setDepthState(GpuObjects::depthReadEq);
	}
}

void Sprite3D::applyBlendmode(rs::GfxContext* _g) {
	Pasta::Graphic* g = _g->gfx;
	Pasta::GraphicContext* ctx = Pasta::GraphicContext::GetCurrent();
	g->setTransparencyType(blendmode);
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

	r2::Lib::applyShaderValues(g, sh, shaderValues);
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
	if (!shouldRenderThisPass(_g)) return false;

	Pasta::Graphic* g = _g->gfx;
	Pasta::GraphicContext* ctx = Pasta::GraphicContext::GetCurrent();
	g->pushContext();

	applyDepth(_g);
	applyBlendmode(_g);

	g->setVertexDeclaration(Pasta::VD_POSITIONS | (hasTexcoords ? Pasta::VD_TEXCOORDS : 0) | (hasNormals ? Pasta::VD_NORMALS : 0));

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

bool Sprite3D::shouldRenderThisPass(rs::GfxContext* _g) {
	if (!_g->supportsEarlyDepth)  return true;
	if (!depthWrite) {
		switch (_g->currentPass) {
		case rs::Pass::EarlyDepth:
			//fallthrough
		case rs::Pass::DepthEqWrite:
			return false;
		case rs::Pass::Basic:
			return true;
		case rs::Pass::Picking:
			return true;
		}
	}

	//else we do Depth Write
	return true;
}

void Sprite3D::im(){
	using namespace ImGui;
	SUPER::im();

	if (!rd::Bits::is(nodeFlags, NF_EDITOR_HIDE_RENDERING) && ImGui::CollapsingHeader(ICON_MD_PALETTE " Sprite 3D")) {
		ImGui::Indent(); ImGui::PushItemWidth(124);

		ImGui::Checkbox("depthRead", &depthRead);
		ImGui::Checkbox("depthWrite", &depthWrite);

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
					shaderValues.head = new Anon();
			}
			ImGui::TreePop();
		}
		ImGui::Unindent();
	}
}

#undef SUPER