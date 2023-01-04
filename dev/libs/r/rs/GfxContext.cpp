#include "stdafx.h"
#include "GfxContext.hpp"
#include "1-graphics/geo_vectors.h"
#include "1-graphics/Graphic.h"
#include "1-graphics/GraphicContext.h"

using namespace rs;
using namespace Pasta;

rs::GfxContext::GfxContext(Pasta::Graphic * pastaGfx) {
	gfx = pastaGfx;
	Pasta::RasterState rs;
	rs.m_cullMode = CullMode::CounterClockwise;
	Pasta::RasterStateID rsid = GraphicContext::CreateRasterStateID(rs);
	pastaGfx->setRasterState(rsid);
	supportsEarlyDepth = false;
	alpha.push_back(1.0);
	setViewport();
}

rs::GfxContext::GfxContext() {
	Pasta::RasterState rs;
	rs.m_cullMode = CullMode::CounterClockwise;
	Pasta::RasterStateID rsid = GraphicContext::CreateRasterStateID(rs);
	supportsEarlyDepth = false;
	alpha.push_back(1.0);
	setViewport();
}

void rs::GfxContext::pushContext(){
	gfx->pushContext();
	Matrix44 oldProj = gfx->getProjMatrix();
	projStack.push_back(oldProj);
	ensureConsistency();
}

void rs::GfxContext::popContext(){
	gfx->popContext();
	Matrix44 proj = projStack[projStack.size() - 1];
	projStack.pop_back();
	gfx->loadProjMatrix(projMatrix=proj);
	ensureConsistency();
}

void GfxContext::pushTarget(FrameBuffer * buff) {
	GraphicContext * ctx = GraphicContext::GetCurrent();

	frameStack.push_back(curBuffer);
	ctx->setFrameBuffer(curBuffer = buff);
	setViewport();
}

void GfxContext::setScissor(int scx, int scy, int scw, int sch) {
	this->scx = scx;
	this->scy = scy;
	this->scw = scw;
	this->sch = sch;
	gfx->setScissor((int)rint(scx), (int)rint(scy), (int)rint(scw), (int)rint(sch));
}

void GfxContext::pushScissor() {
	scissorStack.push_back(Vector4(scx, scy, scw, sch));
}

void GfxContext::popScissor() {
	Vector4 sc = scissorStack.back(); 
	scissorStack.pop_back();
	gfx->setScissor((int)rint(sc.x), (int)rint(sc.y), (int)rint(sc.z), (int)rint(sc.w));

	this->scx = sc.x;
	this->scy = sc.y;
	this->scw = sc.z;
	this->sch = sc.w;
}

void GfxContext::setViewport() {
	GraphicContext* ctx = GraphicContext::GetCurrent();

	ctx->setViewport();
	ctx->getViewport(&this->vpx, &this->vpy, &this->vpw, &this->vph);
}

void GfxContext::setViewport(s16 vpx, s16 vpy, u16 vpw, u16 vph) {
	this->vpx = vpx;
	this->vpy = vpy;
	this->vpw = vpw;
	this->vph = vph;
	gfx->setViewport(vpx, vpy, vpw, vph);
}

void GfxContext::pushViewport() {
	viewportStack.push_back(Vector4(vpx, vpy, vpw, vph));
}

void GfxContext::popViewport() {
	Vector4 vp = viewportStack.back();
	viewportStack.pop_back();
	gfx->setViewport(vp.x, vp.y, vp.z, vp.w);

	this->vpx = vp.x;
	this->vpy = vp.y;
	this->vpw = vp.z;
	this->vph = vp.w;
}

void rs::GfxContext::clear(Pasta::ClearFlags _flags, const r::Color &col, float _depth, u8 _stencil){
	getGpuContext()->clear(_flags,
		Pasta::Color(col.r,col.g,col.b,col.a),
		_depth, _stencil);
}

void rs::GfxContext::push() {
	pushViewMatrix();
	pushProjMatrix();
	pushViewMatrix();
	pushZminmax();
	pushContext();
	pushViewport();
}

void rs::GfxContext::pop() {
	popContext();
	popViewMatrix();
	popProjMatrix();
	popViewMatrix();
	popZminmax();
	popViewport();
}

void rs::GfxContext::loadModelMatrix(const Matrix44 & model)
{
	modelMatrix = model;
	gfx->loadMatrix(viewMatrix*modelMatrix);
}

void rs::GfxContext::loadViewMatrix(const Matrix44 & view){
	viewMatrix = view;
	gfx->loadMatrix(viewMatrix*modelMatrix);
}

void rs::GfxContext::pushViewMatrix() {
	viewStack.push_back(viewMatrix);
}

void rs::GfxContext::popViewMatrix() {
	Matrix44 view = viewStack[viewStack.size() - 1];
	viewStack.pop_back();
	loadViewMatrix(viewMatrix=view);
}

void rs::GfxContext::pushProjMatrix() {
	projStack.push_back(projMatrix);
}

void rs::GfxContext::popProjMatrix() {
	Matrix44 proj = projStack.back();
	projStack.pop_back();
	loadProjMatrix(projMatrix = proj);
}

void rs::GfxContext::loadProjMatrix(const Matrix44 & proj)
{
	projMatrix = proj;
	gfx->loadProjMatrix(projMatrix);
}

void rs::GfxContext::pushZminmax()
{
	zminmaxStack.push_back(Vector2(zMin,zMax));
}

void rs::GfxContext::popZminmax()
{
	Vector2 Zminmax = zminmaxStack.back();
	zminmaxStack.pop_back();
	zMin = Zminmax.x;
	zMax = Zminmax.y;
}

Pasta::GraphicContext * rs::GfxContext::getGpuContext(){
	return GraphicContext::GetCurrent();
}

void GfxContext::popTarget(){
	GraphicContext * ctx = GraphicContext::GetCurrent();

	FrameBuffer * fb = frameStack.back(); frameStack.pop_back();
	ctx->setFrameBuffer(curBuffer = fb);
	setViewport();
}

void rs::GfxContext::ensureConsistency() {
	getGpuContext()->setFrameBuffer(curBuffer);
}


const char* PassNames[(u32)rs::Pass::PassCount] = {
	"EarlyDepth", "DepthEqWrite", "Basic", "Picking"
};
void rs::GfxContext::im() {
	ImGui::Text("Pass: %s", PassNames[(u32)currentPass]);
	ImGui::Text("Draw call: %d", nbDraw);
	ImGui::Text("Nb BatchElem drawn: %d", nbBatchElemDrawn);
}

void rs::GfxContext::resetMetrics() {
	nbBatchElemDrawn = 0;
	nbDraw = 0;
}