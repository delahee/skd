#include "stdafx.h"

#include "1-graphics/geo_vectors.h"
#include "1-graphics/GraphicEnums.h"
#include "1-graphics/ShaderProgram.h"
#include "1-graphics/ShaderParam.h"

#include "Graphics.hpp"
#include "VertexArray.hpp"

using namespace std;
using namespace Pasta;

using namespace rs;
using namespace r2;

r2::VertexArray::VertexArray(r2::Node * parent) : Super(parent){
	setName("VertexArray");
}

r2::VertexArray::~VertexArray() {
	if(saved)
		delete saved;
	saved = nullptr;
	dispose();
}

void r2::VertexArray::reset() {
	Super::reset();
	clear();
	if (saved)
		delete saved;
	saved = nullptr;
}

void r2::VertexArray::dispose() {
	Super::dispose();
	clear();
	if (saved)
		delete saved;
	saved = nullptr;
}

void r2::VertexArray::clear() {
	fbuf.clear();
	vertices.clear();
}

Node* r2::VertexArray::clone(Node* newInst){
	if (!newInst) newInst = new VertexArray();
	r2::VertexArray* newGfx = dynamic_cast<r2::VertexArray*>(newInst);
	Super::clone(newInst);
	if (newGfx) { 
		newGfx->fbuf = fbuf;
		newGfx->vertices = vertices;
	}
	return newInst;
}

void r2::VertexArray::add(const r2::Vertex& vtx){
	vertices.push_back(vtx);
}

bool r2::VertexArray::drawPrepare(rs::GfxContext * _g) {
	if ( !vertices.size() )
		return false;
	
	if (!shouldRenderThisPass(_g)) 
		return false;

	Pasta::Texture* texture = tex;
	if (!texture)
		texture = r2::GpuObjects::whiteTex;

	Pasta::Graphic * g = _g->gfx;
	Pasta::GraphicContext * ctx = Pasta::GraphicContext::GetCurrent();

	g->pushContext();

	applyDepth(_g);
	bindTexture(_g, texture);

	g->setTransparencyType(blendmode);
	g->applyContextTransparency();
	g->setVertexDeclaration( VD_POSITIONS | VD_TEXCOORDS | VD_COLORS );
	g->setColor(Pasta::Color(color.r, color.g, color.b ));
	float finalAlpha = color.a * _g->alpha.back();
	g->setAlpha(finalAlpha);
	if (finalAlpha <= 0.0f) { // early skip this
		g->popContext();
		return false;
	}

	bindShader(_g);

	if (trsDirty && !(nodeFlags & NF_MANUAL_MATRIX)) syncMatrix();
	_g->loadModelMatrix(mat);

	return true;
}

void r2::VertexArray::drawSubmitGeometry(rs::GfxContext * _g) {
	if (trsDirty) 
		syncMatrix();

	auto g = _g->gfx;
	const int stride = 3 + 2 + 4;
	u32 nbVertex = vertices.size();
	u32 byteSize = nbVertex * stride * 4;
	if(fbuf.size()< byteSize + 4)
		fbuf.resize(byteSize + 4);
	float * vertexData = fbuf.data();
	memset(vertexData, 0, byteSize + 4);//we could theoretically remove the memset for optimizations
	int i = 0;
	for (auto & vert : vertices) {
		vertexData[i	 ]	= vert.pos.x;
		vertexData[i + 1 ]	= vert.pos.y;
		vertexData[i + 2 ]	= vert.pos.z;

		vertexData[i + 3 ]	= vert.uv.x;
		vertexData[i + 4 ]	= vert.uv.y;

		vertexData[i + 5 ]	= vert.col.r;
		vertexData[i + 6 ]	= vert.col.g;
		vertexData[i + 7 ]	= vert.col.b;
		vertexData[i + 8 ]	= vert.col.a;

		i += stride;
	}

	if (nbVertex) {
		int nbElem = 0;
		if (primType == Pasta::PrimitiveType::Triangles)
			nbElem = lrint(vertices.size() / 3);
		g->drawPrimitives(primType, nbElem, vertexData);
	}
}

Bounds	r2::VertexArray::getMyLocalBounds() {
	Bounds b = Super::getMyLocalBounds();

	b.empty();
	syncMatrix();

	Pasta::Matrix44 local = getLocalMatrix();
	for ( auto & t : vertices ) {
		Pasta::Vector2 t0 = local * Pasta::Vector2(t.pos.x, t.pos.y);
		b.addPoint(t0);
	}
	return b;
}

void r2::VertexArray::saveState() {
	saved = new eastl::vector<Vertex>();
	*saved = vertices;
}

void r2::VertexArray::restoreState(bool andDelete){
	if (!saved)return;
	vertices = *saved;
	if (andDelete) {
		delete saved;
		saved = nullptr;
	}
}

const eastl::vector<Vertex>& r2::VertexArray::getVertices() const{
	return vertices;
}

Tile* r2::VertexArray::getPrimaryTile(){
	tile.mapTexture(tex);
	return &tile;
}

void r2::VertexArray::serialize(Pasta::JReflect& jr, const char* _name) {
	if (rd::Bits::is(nodeFlags, NF_SKIP_SERIALIZATION))
		return;
	if (_name) jr.visitObjectBegin(_name);
	r2::Sprite::serialize(jr, nullptr);

	Pasta::u32 nbVerts= vertices.size();
	jr.visit(nbVerts, "nbVerts");
	if (jr.m_read && nbVerts) vertices.resize(nbVerts);

	//because visitArray doesn't like nested anymous objects
	if (jr.visitArrayBegin("vertices", nbVerts)) {
		for (Pasta::u32 i = 0; i < nbVerts; ++i) {
			jr.visitIndexBegin(i);
			jr.visitObjectBegin(nullptr);
			jr.visit(vertices[i], nullptr);
			jr.visitObjectEnd(_name, true);
			jr.visitIndexEnd();
		}
	}
	jr.visitArrayEnd("vertices");

	if (_name) jr.visitObjectEnd(_name, true);
};

void r2::VertexArray::im() {
	static int		gfxShapes = 0;
	static Vector4	gfxSize = { 50,5,0,0 };
	static Vector4	gfxP0;
	static r::Color gfxColor;

	using namespace ImGui;
	if (ImGui::CollapsingHeader(ICON_MD_MULTILINE_CHART " VertexArray")) {
		ImGui::Indent();

		SetNextItemWidth(276);
		if (TreeNode("Vertices")) {
			for (int i = 0; i < vertices.size(); i++) {
				PushID(i);
				ImGui::Text("%d.", i);
				Indent();
				PushItemWidth(220);
				DragFloat3("pos", vertices[i].pos.ptr());
				DragFloat2("uv", vertices[i].uv.ptr(),0.1f);
				ColorEdit4("col", vertices[i].col.ptr());
				PopItemWidth();

				if( i != 0)
				if (Button("^")) {
					std::swap(vertices[i - 1], vertices[i]);
					Unindent();
					PopID();
					break;
				}

				if (i <= vertices.size()-2)
					if (Button("v")) {
						std::swap(vertices[i], vertices[i+1]);
						Unindent();
						PopID();
						break;
					}

				Unindent();
				PopID();
			}
			ImGui::Text("Nb vertices : %lu", vertices.size());
			TreePop();
		}
		
		if (ImGui::Button(ICON_MD_CLEAR " Clear")) 
			clear();

		ImGui::Unindent();
	}

	r2::Sprite::im();
}

