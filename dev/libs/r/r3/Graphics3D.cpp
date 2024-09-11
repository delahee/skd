#include "stdafx.h"

#include "Graphics3D.hpp"

#include "1-graphics/geo_vectors.h"
#include "1-graphics/GraphicEnums.h"
#include "1-graphics/ShaderProgram.h"
#include "1-graphics/ShaderParam.h"

#include "r2/Bounds.hpp"
#include "rd/JSerialize.hpp"

using namespace std;
using namespace Pasta;
using namespace rs;
using namespace r3;

r3::Graphics3D::Graphics3D(r2::Node* n) : Super(n) {
	hasVertexColors = true;
}

void r3::Graphics3D::saveState() {
	saved = new eastl::vector<Tri>();
	*saved = triangles;
}

void r3::Graphics3D::restoreState(bool andDelete){
	if (!saved)return;
	triangles = *saved;
	if (andDelete) {
		delete saved;
		saved = nullptr;
	}
}

void r3::Graphics3D::drawSubmitGeometry(rs::GfxContext* _g) {
	if (trsDirty) syncMatrix();

	auto g = _g->gfx;
	const int stride = 3 + 4;
	u32 nbVertex = triangles.size() * 3;
	u32 nbTriangles = triangles.size();

	u32 byteSize = nbVertex * stride * 4;

	if (fbuf.size() < byteSize + 4)
		fbuf.resize(byteSize + 4);

	float* vertexData = fbuf.data();
	memset(vertexData, 0, byteSize + 4);//we could theoretically remove the memset for optimizations

	int i = 0;
	for (auto& tri : triangles) {
		int triStart = i * stride * 3;
		int pos0 = triStart;
		int pos1 = triStart + stride;
		int pos2 = triStart + stride * 2;

		vertexData[pos0] = tri.p0.x;
		vertexData[pos0 + 1] = tri.p0.y;
		vertexData[pos0 + 2] = tri.p0.z;

		vertexData[pos0 + 3] = tri.p0col.r;
		vertexData[pos0 + 4] = tri.p0col.g;
		vertexData[pos0 + 5] = tri.p0col.b;
		vertexData[pos0 + 6] = tri.p0col.a;

		vertexData[pos1] = tri.p1.x;
		vertexData[pos1 + 1] = tri.p1.y;
		vertexData[pos1 + 2] = tri.p1.z;

		vertexData[pos1 + 3] = tri.p1col.r;
		vertexData[pos1 + 4] = tri.p1col.g;
		vertexData[pos1 + 5] = tri.p1col.b;
		vertexData[pos1 + 6] = tri.p1col.a;

		vertexData[pos2] = tri.p2.x;
		vertexData[pos2 + 1] = tri.p2.y;
		vertexData[pos2 + 2] = tri.p2.z;

		vertexData[pos2 + 3] = tri.p2col.r;
		vertexData[pos2 + 4] = tri.p2col.g;
		vertexData[pos2 + 5] = tri.p2col.b;
		vertexData[pos2 + 6] = tri.p2col.a;

		i++;
	}

	if (nbTriangles)
		g->drawPrimitives(PT_TRIANGLES, nbTriangles, vertexData);
}

const eastl::vector<r3::Graphics3D::Tri>& r3::Graphics3D::getTriangles() const {
	return triangles;
}



void Graphics3D::setGeomColor(const r::Color& c) {
	geomColor = c;
}

void Graphics3D::setGeomColor(int col, float alpha) {
	geomColor = r2::Lib::intToColor24(col);
	geomColor.a = alpha;
}

void r3::Graphics3D::dispose() {
	Super::dispose();
	hasVertexColors = true;
	clear();
}

void r3::Graphics3D::reset() {
	Super::reset();
	hasVertexColors = true;
	clear();
}

r2::Bounds r3::Graphics3D::getMyLocalBounds(){
	r2::Bounds b = Super::getMyLocalBounds();

	b.empty();
	syncMatrix();

	Pasta::Matrix44 local = getLocalMatrix();
	for (auto& t : triangles) {
		Pasta::Vector2 t0 = local * Pasta::Vector2(t.p0.x, t.p0.y);
		Pasta::Vector2 t1 = local * Pasta::Vector2(t.p1.x, t.p1.y);
		Pasta::Vector2 t2 = local * Pasta::Vector2(t.p2.x, t.p2.y);
		b.addPoint(t0);
		b.addPoint(t1);
		b.addPoint(t2);
	}
	return b;
};

void r3::Graphics3D::serialize(Pasta::JReflect& jr, const char* _name) {
	if (rd::Bits::is(nodeFlags, NF_SKIP_SERIALIZATION))
		return;
	if (_name) jr.visitObjectBegin(_name);
	r3::Sprite3D::serialize(jr, nullptr);
	jr.visit(geomColor, "geomColor");

	Pasta::u32 nbTri = triangles.size();
	jr.visit(nbTri, "nbTri");
	if (jr.m_read && nbTri) triangles.resize(nbTri);

	//because visitArray doesn't like nested anymous objects
	if (jr.visitArrayBegin("triangles", nbTri)) {
		for (Pasta::u32 i = 0; i < nbTri; ++i) {
			jr.visitIndexBegin(i);
			jr.visitObjectBegin(nullptr);
			jr.visit(triangles[i], nullptr);
			jr.visitObjectEnd(_name, true);
			jr.visitIndexEnd();
		}
	}
	jr.visitArrayEnd("triangles");

	if (_name) jr.visitObjectEnd(_name, true);
};

void Graphics3D::drawHollowRect(float x0, float y0, float x1, float y1, float lineThickness) {
	//   |--t----
	// y0|      |
	//   l      |
	//   |      r
	// y1|      |
	//   ----b--|
	//    x0   x1
	float htick = lineThickness / 2.0f;
	drawLine(x0 - htick, y0 - lineThickness, x0 - htick, y1, lineThickness);//l
	drawLine(x0, y0 - htick, x1 + lineThickness, y0 - htick, lineThickness);//t
	drawLine(x1 + htick, y0, x1 + htick, y1 + lineThickness, lineThickness);//r
	drawLine(x0 - lineThickness, y1 + htick, x1, y1 + htick, lineThickness);//b
}

void Graphics3D::drawTriangleDoubleSided(const Tri& tri) {
	triangles.push_back(tri);
	triangles.push_back(tri.unwind());
}

void Graphics3D::draw3DCircle( const r::Vector3& vtx, const r::Vector3& _up, const r::Vector3& _normal, float radius, const r::Color& col, int nbSegments, float thicc) {
	if (nbSegments <= 0)	nbSegments = std::ceil(radius * 3.14f * 2 / 4);
	if (nbSegments < 3)		nbSegments = 3;

	Vector3 normal = _normal.getNormalized();
	Vector3 up = _up.getNormalized();
	Vector3 front = _up.cross(_normal).getNormalized();

	const float angle = PASTA_PI * 2.0f / nbSegments;

	auto ogc = geomColor;
	geomColor = col;
	for (int i = 0; i < nbSegments; i++) {
		auto a0 = i * angle;
		auto a1 = (i + 1) * angle;

		//auto v = vtx + normal * radius;
		Vector3 v = normal;
		Vector3 r0 = v.getAroundAxisRotated(Vector3(0, 0, 0), up, r::Math::rad2Deg(a0));
		Vector3 r1 = v.getAroundAxisRotated(Vector3(0, 0, 0), up, r::Math::rad2Deg(a1));

		Vector3 r0r = vtx + r0 * radius;
		Vector3 r1r = vtx + r1 * radius;
		drawLine( r0r, r1r, thicc);
	}
	geomColor = ogc;
}

void Graphics3D::drawLine(const Vector3& start, const Vector3& end, float lineThickness) {
	float x0 = start.x;
	float y0 = start.y;
	float x1 = end.x;
	float y1 = end.y;

	float angle = std::atan2(y1 - y0, x1 - x0);
	float hthic = lineThickness * 0.5f;

	float tlX = x0 + std::cos(angle - PASTA_PI * 0.5f) * hthic;
	float tlY = y0 + std::sin(angle - PASTA_PI * 0.5f) * hthic;

	float blX = x0 + std::cos(angle + PASTA_PI * 0.5f) * hthic;
	float blY = y0 + std::sin(angle + PASTA_PI * 0.5f) * hthic;

	float trX = x1 + std::cos(angle - PASTA_PI * 0.5f) * hthic;
	float trY = y1 + std::sin(angle - PASTA_PI * 0.5f) * hthic;

	float brX = x1 + std::cos(angle + PASTA_PI * 0.5f) * hthic;
	float brY = y1 + std::sin(angle + PASTA_PI * 0.5f) * hthic;

	Tri p0;
	p0.setP0(Vector3(tlX, tlY, start.z), geomColor);
	p0.setP1(Vector3(trX, trY, start.z), geomColor);
	p0.setP2(Vector3(blX, blY, end.z), geomColor);

	Tri p1;
	p1.setP0(Vector3(trX, trY, start.z), geomColor);
	p1.setP1(Vector3(brX, brY, end.z), geomColor);
	p1.setP2(Vector3(blX, blY, end.z), geomColor);

	triangles.push_back(p0);
	triangles.push_back(p1);
}




void Graphics3D::drawLine(float x0, float y0, float x1, float y1, float lineThickness) {
	float angle = std::atan2(y1 - y0, x1 - x0);
	float hthic = lineThickness * 0.5;

	float tlX = x0 + std::cos(angle - PASTA_PI * 0.5f) * hthic;
	float tlY = y0 + std::sin(angle - PASTA_PI * 0.5f) * hthic;

	float blX = x0 + std::cos(angle + PASTA_PI * 0.5f) * hthic;
	float blY = y0 + std::sin(angle + PASTA_PI * 0.5f) * hthic;

	float trX = x1 + std::cos(angle - PASTA_PI * 0.5f) * hthic;
	float trY = y1 + std::sin(angle - PASTA_PI * 0.5f) * hthic;

	float brX = x1 + std::cos(angle + PASTA_PI * 0.5f) * hthic;
	float brY = y1 + std::sin(angle + PASTA_PI * 0.5f) * hthic;

	drawTriangle(tlX, tlY, trX, trY, blX, blY);
	drawTriangle(trX, trY, brX, brY, blX, blY);
}

void  Graphics3D::drawQuad(const Quad& q) {
	drawTriangle(q.tri0());
	drawTriangle(q.tri1());
}

void Graphics3D::drawTriangle(const Tri& tri) {
	triangles.push_back(tri);
}

void r3::Graphics3D::im(){
	Super::im();
}

void Graphics3D::drawTriangle(float x0, float y0, float x1, float y1, float x2, float y2) {
	Tri p;

	p.p0.x = x0;
	p.p0.y = y0;
	p.p1.x = x1;
	p.p1.y = y1;
	p.p2.x = x2;
	p.p2.y = y2;

	p.p0col = p.p1col = p.p2col = geomColor;

	triangles.push_back(p);
}

void Graphics3D::clear() {
	fbuf.clear();
	triangles.clear();
	setGeomColor(r::Color(1, 1, 1, 1));
}
