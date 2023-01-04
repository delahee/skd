#include "stdafx.h"

#include "1-graphics/Graphic.h"
#include "1-graphics/GraphicEnums.h"

#include "r2/Patch.hpp"
#include "rs/GfxContext.hpp"
#include "r2/Bounds.hpp"
#include "r2/Sprite.hpp"

using namespace Pasta;
using namespace r2;

static inline const int stride = 5;

void r2::Patch::process(rs::GfxContext* g) {
	for (auto& p : elems) {
		Tile* tile = &p.t;

		const float zTop = useSimpleZ ? 0.0f : zTopOffset;
		const float zBottom = useSimpleZ ? 0.0f : zBottomOffset;

		Vector3 topLeft( p.pos + Vector3(tile->dx, tile->dy, zTop));
		Vector3 topRight(p.pos + Vector3(tile->dx + tile->width, tile->dy, zTop));
		Vector3 bottomLeft(p.pos + Vector3(tile->dx, tile->dy + tile->height, zBottom));
		Vector3 bottomRight(p.pos + Vector3(tile->dx + tile->width, tile->dy + tile->height, zBottom));

		const float u1 = tile->u1;
		const float v1 = tile->v1;
		const float u2 = tile->u2;
		const float v2 = tile->v2;
		const u32 offsetIndex = vbuf.size() / stride;

#define ADD(v)				vbuf.push_back(v);
#define ADD_POS(x,y,z)		ADD(x); ADD(y); ADD(z);
#define ADD_UV(u,v)			ADD(u); ADD(v);

		ADD_POS(topLeft.x, topLeft.y, topLeft.z);
		ADD_UV(u1, v1);

		ADD_POS(topRight.x, topRight.y, topRight.z);
		ADD_UV(u2, v1);

		ADD_POS(bottomLeft.x, bottomLeft.y, bottomLeft.z);
		ADD_UV(u1, v2);

		ADD_POS(bottomRight.x, bottomRight.y, bottomRight.z);
		ADD_UV(u2, v2);

#define ADD_INDEX(v)		ibuf.push_back(v + offsetIndex);
#define ADD_TRI(a,b,c)		ADD_INDEX(a); ADD_INDEX(b); ADD_INDEX(c);

		ADD_TRI(0, 1, 2);
		ADD_TRI(2, 1, 3);
	}
}

void r2::Patch::setTransparency(rs::GfxContext* _g) {
	auto g = _g->gfx;
	switch (_g->currentPass) {
	case rs::Pass::EarlyDepth:
		g->setBlendState(r2::GpuObjects::opaqueNoColor);
		return;
	case rs::Pass::Picking:
		g->setTransparencyType(TransparencyType::TT_CLIP_ALPHA);
		break;
	default:
		g->setTransparencyType(blendmode);
		break;
	}
	g->applyContextTransparency();
}

Texture* r2::Patch::getTexture(rs::GfxContext* _g, int slot) {
	if (_g)
		return _g->getGpuContext()->getTexture(Pasta::ShaderStage::Fragment, slot);
	else
		if (elems.size()) 
			return elems[0].t.getTexture();
		else
			return nullptr;
}

void r2::Patch::draw(rs::GfxContext* _g){
	if (elems.size() == 0)
		return;

	if (_g->supportsEarlyDepth) {
		switch (_g->currentPass) { // the rest is handled by should render
		case rs::Pass::EarlyDepth:		//fallthrough
		case rs::Pass::DepthEqWrite:
			if (!depthWrite) return;
			break;
		case rs::Pass::Basic:			//do nothing
		case rs::Pass::Picking:
			break;
		}
	}
	else {
		switch (_g->currentPass) {
		case rs::Pass::Picking:
			if (nodeFlags & (NF_UTILITY | NF_EDITOR_FROZEN | NF_EDITOR_PROTECT)) return;
			break;
		}
	}

	if (!shouldRender(_g))
		return;

	Pasta::Graphic* g = _g->gfx;
	Pasta::GraphicContext* ctx = Pasta::GraphicContext::GetCurrent();

	//prepare sizing
	const int verts = 4;
	const int tri = 2;
	const int count = elems.size();

	vbuf.clear();
	if (vbuf.capacity() < count * verts * stride)
		vbuf.reserve(count * verts * stride);
	ibuf.clear();
	if (ibuf.capacity() < count * tri * 2)
		ibuf.reserve(count * tri * 2);

	process(_g);

	g->pushContext();

	if (trsDirty && !(nodeFlags & NF_MANUAL_MATRIX)) syncMatrix();
	_g->loadModelMatrix(mat);
	applyDepth(_g);
	g->setVertexDeclaration(VD_POSITIONS | VD_TEXCOORDS);
	Pasta::Color c(color.r, color.g, color.b, color.a * alpha * _g->alpha.back());
	g->setColor(c);
	g->setAlpha(c.a);

	bindTexture(_g, elems[0].t.getTexture(), 0);
	bindShader(_g);
	setTransparency(_g);
	{
		g->drawIndexed(PrimitiveType::Triangles, vbuf.data(), vbuf.size() / stride, ibuf.data(), ibuf.size());
	}

	//clean
	g->setVertexDeclaration(0);
	g->setTexture(Pasta::ShaderStage::Fragment, 0, NULL);
	g->popContext();
}

void r2::Patch::add(const PatchElem& e){
	elems.push_back(e);
}


bool r2::Patch::shouldRender(rs::GfxContext* _g) {
	if (!_g->supportsEarlyDepth) {
		return
			visible
			&& ((alpha * color.a) > 0);
	}
	else {
		if (!(visible && ((alpha * color.a) > 0)))
			return false;

		auto blend = blendmode;
		if (blend == r::TransparencyType::TT_INHERIT)
			blend = r::TransparencyType::TT_ALPHA;

		const bool earlyDepthCompatible = blend == r::TransparencyType::TT_CLIP_ALPHA || blend == r::TransparencyType::TT_OPAQUE;
		switch (_g->currentPass) {
		case rs::Pass::EarlyDepth:		// fallthrough
		case rs::Pass::DepthEqWrite:	return earlyDepthCompatible;
		case rs::Pass::Basic:			return !earlyDepthCompatible;
		case rs::Pass::Picking:			return true; 
		default: 						return true;
		};
	}
}

r2::Bounds r2::Patch::getMyLocalBounds(){
	Bounds b;
	b.empty();
	syncAllMatrix();
	Pasta::Matrix44 local = getLocalMatrix();
	for (auto &e : elems) {
		auto& t = e.t;
		Pasta::Matrix44 local = getLocalMatrix();
		Pasta::Vector3 topLeft = local * (e.pos + Pasta::Vector3(t.dx, t.dy, 0.0));
		Pasta::Vector3 topRight = local * (e.pos + Pasta::Vector3(t.dx + t.width, t.dy, 0.0));
		Pasta::Vector3 bottomLeft = local * (e.pos + Pasta::Vector3(t.dx, t.dy + t.height, 0.0));
		Pasta::Vector3 bottomRight = local * (e.pos + Pasta::Vector3(t.dx + t.width, t.dy + t.height, 0.0));
		b.addPoint(topLeft.x, topLeft.y);
		b.addPoint(topRight.x, topRight.y);
		b.addPoint(bottomLeft.x, bottomLeft.y);
		b.addPoint(bottomRight.x, bottomRight.y);
	}
	return b;
}

void r2::Patch::im(){
	using namespace ImGui;
	r2::Sprite::im();
	int idx = 0;
	for (auto& e : elems) {
		if( TreeNode(std::to_string(idx))){
			DragFloat3("pos", e.pos.ptr());
			e.t.im();
			TreePop();
		}
		idx++;
	}
}
