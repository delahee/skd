#include "stdafx.h"

#include "1-graphics/geo_vectors.h"
#include "1-graphics/GraphicEnums.h"
#include "1-graphics/ShaderProgram.h"
#include "1-graphics/ShaderParam.h"

#include "Graphics.hpp"

#include "ext/ExtGraphics.hpp"

using namespace std;
using namespace Pasta;

using namespace rs;
using namespace r2;

#define SUPER Sprite

void r2::Tri::setP0(const Vector3 &p, r::Color color) {
	p0.x = p.x;
	p0.y = p.y;
	p0.z = p.z;
	p0col = color;
}

void r2::Tri::setP1(const Vector3 &p, r::Color color) {
	p1.x = p.x;
	p1.y = p.y;
	p1.z = p.z;
	p1col = color;
}
void r2::Tri::setP2(const Vector3 &p, r::Color color) {
	p2.x = p.x;
	p2.y = p.y;
	p2.z = p.z;
	p2col = color;
}

void r2::Tri::setP0(float x, float y, float z, r::Color color) {
	p0.x = x;
	p0.y = y;
	p0.z = z;
	p0col = color;
}

void r2::Tri::setP1(float x, float y, float z, r::Color color) {
	p1.x = x;
	p1.y = y;
	p1.z = z;
	p1col = color;
}

void r2::Tri::setP2(float x, float y, float z, r::Color color){
	p2.x = x;
	p2.y = y;
	p2.z = z;
	p2col = color;
}

string r2::Tri::toString()
{
	string res;
	res += string("p0:") + to_string(p0.x)+","+to_string(p0.y)+","+to_string(p0.z) + "\n";
	res += string("c0:") + p0col.toHexString() + "\n";

	res += string("p1:") + to_string(p1.x)+","+to_string(p1.y)+","+to_string(p1.z) + "\n";
	res += string("c1:") + p1col.toHexString() + "\n";

	res += string("p2:") + to_string(p2.x)+","+to_string(p2.y)+","+to_string(p2.z) + "\n";
	res += string("c2:") + p2col.toHexString() + "\n";
	return res;
}

Tri::Tri(const Tri&ot) {
	p0.x = ot.p0.x;
	p0.y = ot.p0.y;
	p0.z = ot.p0.z;
	  
	p1.x = ot.p1.x;
	p1.y = ot.p1.y;
	p1.z = ot.p1.z;
	  
	p2.x = ot.p2.x;
	p2.y = ot.p2.y;
	p2.z = ot.p2.z;

	p0col = ot.p0col;
	p1col = ot.p1col;
	p2col = ot.p2col;
}

Tri r2::Tri::unwind() const{
	auto t = Tri( *this );
	std::swap(t.p0.x, t.p1.x);
	std::swap(t.p0.y, t.p1.y);
	std::swap(t.p0.z, t.p1.z);
	std::swap(t.p0col, t.p1col);
	return t;
}

void r2::Quad::setP0(const Vector3& p, r::Color color) {
	p0x = p.x;
	p0y = p.y;
	p0z = p.z;
	p0col = color;
}

void r2::Quad::setP1(const Vector3& p, r::Color color) {
	p1x = p.x;
	p1y = p.y;
	p1z = p.z;
	p1col = color;
}
void r2::Quad::setP2(const Vector3& p, r::Color color) {
	p2x = p.x;
	p2y = p.y;
	p2z = p.z;
	p2col = color;
}
void r2::Quad::setP3(const Vector3& p, r::Color color) {
	p3x = p.x;
	p3y = p.y;
	p3z = p.z;
	p3col = color;
}

void r2::Quad::setP0(float x, float y, float z, r::Color color) {
	p0x = x;
	p0y = y;
	p0z = z;
	p0col = color;
}

void r2::Quad::setP1(float x, float y, float z, r::Color color) {
	p1x = x;
	p1y = y;
	p1z = z;
	p1col = color;
}

void r2::Quad::setP2(float x, float y, float z, r::Color color) {
	p2x = x;
	p2y = y;
	p2z = z;
	p2col = color;
}

void r2::Quad::setP3(float x, float y, float z, r::Color color) {
	p3x = x;
	p3y = y;
	p3z = z;
	p3col = color;
}

Tri r2::Quad::tri0() const {
	Tri t;
	t.setP0(p0x, p0y, p0z, p0col);
	t.setP1(p1x, p1y, p1z, p1col);
	t.setP2(p2x, p2y, p2z, p2col);
	return t;
}

Tri r2::Quad::tri1() const {
	Tri t;
	t.setP0(p2x, p2y, p2z, p2col);
	t.setP1(p1x, p1y, p1z, p1col);
	t.setP2(p3x, p3y, p3z, p3col);
	return t;
}

string r2::Quad::toString()
{
	string res;
	res += string("p0:") + to_string(p0x) + "," + to_string(p0y) + "," + to_string(p0z) + "\n";
	res += string("c0:") + p0col.toHexString() + "\n";

	res += string("p1:") + to_string(p1x) + "," + to_string(p1y) + "," + to_string(p1z) + "\n";
	res += string("c1:") + p1col.toHexString() + "\n";

	res += string("p2:") + to_string(p2x) + "," + to_string(p2y) + "," + to_string(p2z) + "\n";
	res += string("c2:") + p2col.toHexString() + "\n";

	res += string("p3:") + to_string(p3x) + "," + to_string(p3y) + "," + to_string(p3z) + "\n";
	res += string("c3:") + p3col.toHexString() + "\n";
	return res;
}

r2::Graphics::Graphics(r2::Node * parent) : SUPER(parent){
	name = string("graphics#") + to_string(uid);
}

r2::Graphics::~Graphics() {
	dispose();
	if(saved)
		delete saved;
	saved = nullptr;
}

void r2::Graphics::dispose() {
	SUPER::dispose();
	clear();
	if (saved)
		delete saved;
	saved = nullptr;
	setGeomColor(r::Color(1, 1, 1, 1));
}

void r2::Graphics::clear() {
	fbuf.clear();
	triangles.clear();
	setGeomColor(r::Color(1, 1, 1, 1));
}

void r2::Graphics::drawLine(const Vector3& start, const Vector3& end, float lineThickness){
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
	p0.setP0(Vector3(tlX, tlY, start.z),geomColor);
	p0.setP1(Vector3(trX, trY, start.z), geomColor);
	p0.setP2(Vector3(blX, blY, end.z), geomColor);

	Tri p1;
	p1.setP0(Vector3(trX, trY, start.z), geomColor);
	p1.setP1(Vector3(brX, brY, end.z), geomColor);
	p1.setP2(Vector3(blX, blY, end.z), geomColor);

	triangles.push_back(p0);
	triangles.push_back(p1);
}

void r2::Graphics::drawLine(float x0, float y0, float x1, float y1, float lineThickness){
	float angle = std::atan2(y1 - y0, x1 - x0);
	float hthic = lineThickness*0.5;

	float tlX = x0 + std::cos(angle - PASTA_PI * 0.5f) * hthic;
	float tlY = y0 + std::sin(angle - PASTA_PI * 0.5f) * hthic;

	float blX = x0 + std::cos(angle + PASTA_PI * 0.5f) * hthic;
	float blY = y0 + std::sin(angle + PASTA_PI * 0.5f) * hthic;

	float trX = x1 + std::cos(angle - PASTA_PI * 0.5f) * hthic;
	float trY = y1 + std::sin(angle - PASTA_PI * 0.5f) * hthic;

	float brX = x1 + std::cos(angle + PASTA_PI * 0.5f) * hthic;
	float brY = y1 + std::sin(angle + PASTA_PI * 0.5f) * hthic;

	drawTriangle(tlX, tlY, trX, trY,blX,blY );
	drawTriangle(trX,trY, brX,brY, blX, blY);
}

void r2::Graphics::drawTriangle(float x0, float y0, float x1, float y1, float x2, float y2){
	Tri p;

	p.p0.x	= x0;
	p.p0.y	= y0;
	p.p1.x	= x1;
	p.p1.y	= y1;
	p.p2.x	= x2;
	p.p2.y	= y2;

	p.p0col = p.p1col = p.p2col = geomColor;

	triangles.push_back( p );
}

void  r2::Graphics::drawQuad(const Quad& q) {
	drawTriangle(q.tri0());
	drawTriangle(q.tri1());
}

void r2::Graphics::drawTriangle(const Tri & tri) {
	triangles.push_back(tri);
}

void r2::Graphics::drawTriangleDoubleSided(const Tri& tri) {
	triangles.push_back(tri);
	triangles.push_back(tri.unwind());
}

void r2::Graphics::drawQuad(float x0, float y0, float x1, float y1) {
	/*
	0--0
	|
	0

	--1
	|
	1--1
	*/
	float top = y0;
	float left = x0;
	float bottom = y1;
	float right = x1;

	drawTriangle(left, bottom, left, top, right, top);
	drawTriangle(right, top, right, bottom, left, bottom);
}

void r2::Graphics::drawRect(float x0, float y0, float x1, float y1) {
	drawQuad(x0, y0, x1, y1);
}

void r2::Graphics::drawDisc(float x0, float y0, float radius, int nbSegments) {
	if (nbSegments <= 0)	nbSegments = std::ceil(radius * 3.14 * 2 / 4);
	if (nbSegments < 3)		nbSegments = 3;

	float angle = PASTA_PI * 2 / nbSegments;
	for (int i = 0; i < nbSegments; i++) {
		auto a0 = i * angle;
		auto a1 = (i + 1) * angle;

		auto a0x = x0 + std::cos(a0) * radius;
		auto a0y = y0 + std::sin(a0) * radius;

		auto a1x = x0 + std::cos(a1) * radius;
		auto a1y = y0 + std::sin(a1) * radius;

		drawTriangle(
			a0x,a0y,
			a1x,a1y,
			x0, y0
		);
	}
}

void r2::Graphics::setGeomColor(const r::Color & c) {
	geomColor = c;
}

void r2::Graphics::setGeomColor(int col, float alpha) {
	geomColor = r2::Lib::intToColor24(col);
	geomColor.a = alpha;
}

void r2::Graphics::drawCircle(float x0, float y0, float radius, float thicc, int nbSegments)
{
	if (nbSegments <= 0)	nbSegments = std::ceil(radius * 3.14 * 2 / 4);
	if (nbSegments < 3)		nbSegments = 3;

	float angle = PASTA_PI * 2 / nbSegments;
	for (int i = 0; i < nbSegments; i++) {
		auto a0 = i * angle;
		auto a1 = (i+1) * angle;

		auto a0x = x0 + std::cos(a0) * radius;
		auto a0y = y0 + std::sin(a0) * radius;

		auto a1x = x0 + std::cos(a1) * radius;
		auto a1y = y0 + std::sin(a1) * radius;

		drawLine(a0x, a0y, a1x, a1y, thicc);
	}
}

void r2::Graphics::drawCross(float x0, float y0, float radius, float lineThickness)
{
	drawLine(x0-radius, y0, x0+radius, y0, lineThickness);
	drawLine(x0, y0-radius, x0, y0+radius, lineThickness);
}

void r2::Graphics::drawHollowRect(float x0, float y0, float x1, float y1, float lineThickness)
{
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

static eastl::vector<eastl::pair<float, r::Color>> s_gradientBuffer;
void r2::Graphics::drawHorizRectGradient(Vector2 tl, Vector2 dr, r::Color cStart, r::Color cEnd){
	s_gradientBuffer.clear();

	s_gradientBuffer.push_back(eastl::make_pair(0, cStart));
	s_gradientBuffer.push_back(eastl::make_pair(1, cEnd));

	Vector2 delta = dr - tl;
	float thicc = fabsf(delta.y);
	r2::ext::drawGradient(this,
		Vector3(tl.x, tl.y + thicc * 0.5, 0),
		Vector3(dr.x, dr.y - thicc * 0.5, 0),
		s_gradientBuffer, thicc);
}

/**
cannot super call for vertex decls are different
*/
bool r2::Graphics::drawPrepare(rs::GfxContext * _g) {
	if (triangles.size() == 0)
		return false;
	
	if (!shouldRenderThisPass(_g)) return false;

	Pasta::Graphic * g = _g->gfx;
	Pasta::GraphicContext * ctx = Pasta::GraphicContext::GetCurrent();

	g->pushContext();

	applyDepth(_g);

	g->setTransparencyType(blendmode);
	g->applyContextTransparency();

	g->setVertexDeclaration(VD_POSITIONS | VD_COLORS);

	g->setColor(Pasta::Color(color.r, color.g, color.b ));
	float finalAlpha = color.a * _g->alpha.back();
	g->setAlpha(color.a * _g->alpha.back());
	if (finalAlpha <= 0.0f) { // early skip this
		g->popContext();
		return false;
	}

	bindTexture(_g, nullptr);
	bindShader(_g);

	if (trsDirty && !(nodeFlags & NF_MANUAL_MATRIX)) syncMatrix();

	_g->loadModelMatrix(mat);

	return true;
}

void r2::Graphics::drawSubmitGeometry(rs::GfxContext * _g) {
	if (trsDirty) syncMatrix();

	auto g = _g->gfx;
	const int stride = 3 + 4;
	u32 nbVertex = triangles.size() * 3;
	u32 nbTriangles = triangles.size();

	u32 byteSize = nbVertex * stride * 4;

	if(fbuf.size()< byteSize + 4)
		fbuf.resize(byteSize + 4);

	float * vertexData = fbuf.data();
	memset(vertexData, 0, byteSize + 4);//we could theoretically remove the memset for optimizations

	int i = 0;
	for (auto & tri : triangles) {
		int triStart = i * stride * 3;
		int pos0 = triStart;
		int pos1 = triStart + stride;
		int pos2 = triStart + stride*2;

		vertexData[pos0		]	= tri.p0.x;
		vertexData[pos0 + 1	]	= tri.p0.y;
		vertexData[pos0 + 2	]	= tri.p0.z;

		vertexData[pos0 + 3] = tri.p0col.r;
		vertexData[pos0 + 4] = tri.p0col.g;
		vertexData[pos0 + 5] = tri.p0col.b;
		vertexData[pos0 + 6] = tri.p0col.a;
		
		vertexData[pos1		] = tri.p1.x;
		vertexData[pos1 + 1	] = tri.p1.y;
		vertexData[pos1 + 2	] = tri.p1.z;

		vertexData[pos1 + 3 ]= tri.p1col.r;
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

	if(nbTriangles)
		g->drawPrimitives(PT_TRIANGLES, nbTriangles, vertexData);
}

Graphics * r2::Graphics::fromBounds(Bounds bnd, r::Color col, r2::Node * parent )
{
	if ( parent == nullptr)	return nullptr;
	if ( bnd.isEmpty() )	return nullptr;

	auto g = new Graphics(parent);

	g->geomColor = col;
	g->geomColor.r += 0.2f;
	g->geomColor.g += 0.2f;
	g->geomColor.b += 0.2f;
	g->geomColor.a += 0.2f;

	float left		= bnd.left();
	float top		= bnd.top();
	float bottom	= bnd.bottom();
	float right		= bnd.right();

	g->drawQuad( left,top,right,bottom );

	g->geomColor = col;

	g->drawLine(	left,	top,
					right,	top);

	g->drawLine(	right,	bottom,
					right,	top);

	g->drawLine(	left,	bottom,
					left,	top);

	g->drawLine(	left,	bottom,
					right,	bottom);
	return g;
}

Graphics * r2::Graphics::linesFromBounds(Bounds bnd,  r::Color col, float thicc, r2::Node * parent )
{
	if (parent == nullptr)	return nullptr;
	if (bnd.isEmpty())	return nullptr;

	auto g = fromPool(parent);
	
	float left		= bnd.left();
	float top		= bnd.top();
	float bottom	= bnd.bottom();
	float right		= bnd.right();

	g->geomColor = col;
	g->geomColor.r += 0.25;
	g->geomColor.g += 0.25;
	g->geomColor.b += 0.25;
	g->geomColor.a += 0.25;
	g->drawLine(	left, top,
					right, top, thicc);
	g->drawLine(	right, bottom,
					right, top, thicc);

	g->drawLine(	left, bottom,
					left, top, thicc);

	g->drawLine(	left, bottom,
					right, bottom, thicc);
	return g;
}

Graphics * r2::Graphics::rectFromBounds(Bounds bnd, r::Color col,  r2::Node * main){
	if (main == nullptr)	return nullptr;
	if (bnd.isEmpty())	return nullptr;

	auto g = fromPool(main);

	g->geomColor = col;

	float left = bnd.left();
	float top = bnd.top();
	float bottom = bnd.bottom();
	float right = bnd.right();

	g->drawQuad(left, top, right, bottom);
	return g;
}

Graphics* r2::Graphics::fromPool(r2::Node* parent){
	auto g =  rd::Pools::graphics.alloc();
	parent->addChild(g);
	return g;
}

Graphics* r2::Graphics::rect(
	const Vector2& pos,
	const Vector2& size,
	const r::Color& col, r2::Node* parent ) {
	if (parent == nullptr)	return nullptr;

	auto g = fromPool(parent);

	g->geomColor = col;

	float left = pos.x;
	float top = pos.y;
	float bottom = pos.y + size.y;
	float right = pos.x + size.x;

	g->drawQuad(left, top, right, bottom);
	return g;
}

r2::Graphics * r2::Graphics::rect(float x, float y, float width, float height, int color, float alpha, r2::Node * parent)
{
	if (parent == nullptr)	return nullptr;

	auto g = new Graphics(parent);

	g->geomColor = r::Color(color, alpha);

	float left = x;
	float top = y;
	float bottom = y+height;
	float right = x+width;

	g->drawQuad(left, top, right, bottom);
	return g;
}

Bounds	r2::Graphics::getMyLocalBounds() {
	Bounds b = SUPER::getMyLocalBounds();

	b.empty();
	syncMatrix();

	Pasta::Matrix44 local = getLocalMatrix();
	for ( auto & t : triangles) {
		Pasta::Vector2 t0 = local * Pasta::Vector2(t.p0.x, t.p0.y);
		Pasta::Vector2 t1 = local * Pasta::Vector2(t.p1.x, t.p1.y);
		Pasta::Vector2 t2 = local * Pasta::Vector2(t.p2.x, t.p2.y);
		b.addPoint(t0);
		b.addPoint(t1);
		b.addPoint(t2);
	}
	return b;
}

void r2::Graphics::saveState() {
	saved = new eastl::vector<Tri>();
	*saved = triangles;
}

void r2::Graphics::restoreState(bool andDelete)
{
	if (!saved)return;
	triangles = *saved;
	if (andDelete) {
		delete saved;
		saved = nullptr;
	}
}

const eastl::vector<Tri>& r2::Graphics::getTriangles() const{
	return triangles;
}



#undef SUPER