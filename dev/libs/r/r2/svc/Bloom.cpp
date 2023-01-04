#include "stdafx.h"

#include "Bloom.hpp"

using namespace std;
using namespace r2;
using namespace r2::svc;

static string uPass = ("uPass");
static string uMul = ("uMul");
static string uLuminanceVector = ("uLuminanceVector");

Bloom::Bloom() : ctrl(nullptr,nullptr){
}

void r2::svc::Bloom::bmpOpHighPass(r2::Bitmap & bmp)
{
	bmp.mkUber();
	bmp.shaderFlags |= USF_Bloom;
	bmp.updateShaderParam(uPass, ctrl.pass);
	Vector4 cmul = ctrl.mul.toVec4() * ctrl.intensity;
	bmp.updateShaderParam(uMul, cmul.ptr(), 4);
	bmp.updateShaderParam(uLuminanceVector, ctrl.luminanceVector.ptr(), 3);
}

r2::Tile * r2::svc::Bloom::make(r2::Tile * tile)
{
	ctrl.src = tile->getTexture();

	if (ctrl.mul == r::Color(0, 0, 0, 0)) {
		dummy.copy(*tile);
		highPass.rd.isFresh = true;
		workingTile.copy(dummy);
		drawingTile.copy(dummy);
		return &dummy;
	}

	if (ctrl.dx == 0 && ctrl.dy == 0 ) {
		dummy.copy(*tile);
		highPass.rd.isFresh = true;
		workingTile.copy(dummy);
		drawingTile.copy(dummy);
		return &dummy;
	}

	tile->getTexture()->setWrapModeUVW(PASTA_TEXTURE_CLAMP);

	ops.clear();
	ops.push_back([=](r2::Bitmap&bmp) { bmpOpHighPass(bmp); });

	highPass.make(tile, ops);
	r2::Tile* tileHighPass = highPass.rd.getWorkingTile();
	GaussianBlurCtrl gctrl;
	gctrl.offsetScale = r::Vector2(ctrl.offsetScale, ctrl.offsetScale);
	gctrl.filter = ctrl.blurFilter;
	gctrl.size = r::Vector2(ctrl.dx,ctrl.dy);
	gctrl.doPad = ctrl.doPad;
	highPassBlur.make(tileHighPass, gctrl);
	r2::Tile* tileHighPassBlurred = &highPassBlur.workingTile;

	//prepare color
	Node * compoNode = rd::Pools::nodes.alloc();
	Bitmap * bSrc = rd::Pools::bitmaps.alloc();
	Bitmap * bBlur = rd::Pools::bitmaps.alloc();

	bSrc->copyTile( tile );
	bSrc->visible = !ctrl.skipColorPass;
	bSrc->texFiltering = ctrl.renderFilter;

	//prepare hp
	bBlur->copyTile(tileHighPassBlurred);
	bBlur->texFiltering = ctrl.renderFilter;
	bBlur->blendmode = Pasta::TransparencyType::TT_ADD;
	bBlur->visible = !ctrl.skipBloomPass;

	compoNode->addChild(bSrc);
	compoNode->addChild(bBlur);

	composite.make(compoNode, ctrl.renderFilter);
	r2::Tile* result = composite.getWorkingTile();

	bSrc->removeAllChildren();
	bBlur->removeAllChildren();
	compoNode->removeAllChildren();

	rd::Pools::bitmaps.free(bSrc);
	rd::Pools::bitmaps.free(bBlur);
	rd::Pools::nodes.free(compoNode);

	workingTile.copy(*composite.getWorkingTile());
	drawingTile = *composite.rd.getDrawingTile();
	return &drawingTile;
}

r2::Tile * Bloom::make(r::Texture * src){
	stub.mapTexture(src);
	return make(&stub);
};

void r2::svc::Bloom::im()
{
	ImGui::PushID(this);
	
	if (ImGui::CollapsingHeader("Bloom")) {
		ImGui::Indent();
		ctrl.im();
		highPass.im();
		highPassBlur.im();
		composite.im();
		ImGui::Unindent();
	}
	
	ImGui::PopID();
}