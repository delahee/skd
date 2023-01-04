#include "stdafx.h"
#include "r2/ext/ExtGraphics.hpp"

using namespace std;
using namespace r;
using namespace r2;
using namespace r2::ext;

void r2::ext::drawUnitGradient(r2::Graphics* gfx, eastl::vector<eastl::pair<float, r::Color>>& cols) {
	r2::ext::drawGradient(gfx, Vector3(0.5, 0, 0), Vector3(0.5, 1, 0), cols, 1);
}

void r2::ext::drawBoxGradient(
	r2::Graphics* gfx, 
	const r::Vector3& p0, 
	const r::Vector3& p1, 
	eastl::vector<eastl::tuple<float, r::Color, r::Color>>& colors, 
	float thicc) {
	
	if (colors.size() == 0) {
		r2::ext::drawLine(gfx, p0, p1, r::Color::White, r::Color::White, thicc);
		return;
	}
	else if (colors.size() == 1) {
		r2::ext::drawLine(gfx, p0, p1, get<1>(colors[0]), get<1>(colors[0]), thicc);
		return;
	}

	float lineThickness = thicc;
	float x0 = p0.x; float y0 = p0.y;
	float x1 = p1.x; float y1 = p1.y;

	float angle = std::atan2(p1.y - p0.y, p1.x - p0.x);
	float hthic = lineThickness * 0.5;

	float hpi = PASTA_PI * 0.5f;

	float tlX = x0 + std::cos(angle + hpi) * hthic;
	float tlY = y0 + std::sin(angle + hpi) * hthic;
	float tlZ = p0.z;

	float blX = x0 + std::cos(angle + hpi) * hthic;
	float blY = y1 + std::sin(angle + hpi) * hthic;
	float blZ = p1.z;

	float trX = x1 + std::cos(angle - hpi) * hthic;
	float trY = y0 + std::sin(angle - hpi) * hthic;
	float trZ = p0.z;

	float brX = x1 + std::cos(angle - hpi) * hthic;
	float brY = y1 + std::sin(angle - hpi) * hthic;
	float brZ = p1.z;

	//vector<Quad> segments;

	auto find = [&colors](float t0) -> std::optional<eastl::tuple<float,r::Color,r::Color>>{
		for (auto& p : colors) 
			if (r::Math::essentiallyEqual(get<0>(p), t0, 0.0001f))
				return p;
		return nullopt;
	};

	auto add = [=, &colors//, &segments
	](float t0, float t1) {
		float _tlX = Pasta::Lerp(tlX, blX, t0);
		float _tlY = Pasta::Lerp(tlY, blY, t0);
		float _tlZ = Pasta::Lerp(tlZ, blZ, t0);
		float _blX = Pasta::Lerp(tlX, blX, t1);
		float _blY = Pasta::Lerp(tlY, blY, t1);
		float _blZ = Pasta::Lerp(tlZ, blZ, t1);
		float _trX = Pasta::Lerp(trX, brX, t0);
		float _trY = Pasta::Lerp(trY, brY, t0);
		float _trZ = Pasta::Lerp(trZ, brZ, t0);
		float _brX = Pasta::Lerp(trX, brX, t1);
		float _brY = Pasta::Lerp(trY, brY, t1);
		float _brZ = Pasta::Lerp(trZ, brZ, t1);

		std::optional<eastl::tuple<float, r::Color, r::Color>> p0 = find(t0);
		if (!p0)p0 = colors[0];
		std::optional<eastl::tuple<float,r::Color,r::Color>> p1 = find(t1);
		if (!p1)p1 = colors.back();

		r::Color cTopLeft = eastl::get<1>(*p0);
		r::Color cTopRight = eastl::get<2>(*p0);
		r::Color cBottomLeft = eastl::get<1>(*p1);
		r::Color cBottomRight = eastl::get<2>(*p1);

		Quad q;
		q.setP0(_tlX, _tlY, _tlZ, cTopLeft);
		q.setP1(_trX, _trY, _trZ, cTopRight);
		q.setP2(_blX, _blY, _blZ, cBottomLeft);
		q.setP3(_brX, _brY, _brZ, cBottomRight);

		//segments.push_back(q);

		gfx->drawQuad(q);
	};

	if ( get<0>(colors[0]) != 0.0) 
		add(0.0, get<0>(colors[0]));

	for (int i = 0; i < colors.size() - 1; ++i) 
		add( get<0>(colors[i]), get<0>(colors[i + 1]) );

	if (r::Math::essentiallyEqual(get<0>(colors.back()) , 1.0f, 0.001f))
		add(get<0>(colors.back()), 1.0f);
}


void r2::ext::drawBands(r2::Graphics* gfx, const r::Vector3& p0, const r::Vector3& p1, eastl::vector<eastl::pair<float, r::Color>>& colors, float thicc) {
	if (colors.size() <= 0) {
		r2::ext::drawLine(gfx, p0, p1, r::Color::White, r::Color::White, thicc);
		return;
	}
	else if (colors.size() == 1) {
		r2::ext::drawLine(gfx, p0, p1, colors[0].second, colors[0].second, thicc);
		return;
	}

	if (colors[0].first != 0.0) {
		r2::ext::drawLine(gfx,
			p0 + (p1 - p0) * 0.0,
			p0 + (p1 - p0) * colors[0].first,
			colors[0].second,
			colors[0].second, thicc);
	}

	for (int i = 0; i < colors.size()-1; ++i) {
		float t0 = colors[i].first;
		float t1 = colors[i+1].first;
		r2::ext::drawLine(gfx, 
			p0 + (p1-p0) * t0, 
			p0 + (p1 - p0) * t1,
			colors[i].second, 
			colors[i].second, thicc);
	}

	if (colors.back().first != 1.0) {
		r2::ext::drawLine(gfx,
			p0 + (p1 - p0) * colors.back().first,
			p0 + (p1 - p0) * 1.0,
			colors.back().second,
			colors.back().second, thicc);
	}
}

void swapF(float& a, float& b) {
	float c = a;
	a = b;
	b = c;
}

void r2::ext::drawGradient(r2::Graphics* gfx, const r::Vector3& p0, const r::Vector3& p1, eastl::vector<eastl::pair<float, r::Color>>& colors, float thicc) {
	if (colors.size() <= 0) {
		r2::ext::drawLine(gfx, p0, p1, r::Color::White, r::Color::White, thicc);
		return;
	}
	else if (colors.size() == 1) {
		r2::ext::drawLine(gfx, p0, p1, colors[0].second, colors[0].second, thicc);
		return;
	}
	else if (colors.size() == 2) {
		r2::ext::drawLine(gfx, p0, p1, colors[0].second, colors[1].second, thicc);
		return;
	}

	float lineThickness = thicc;
	float x0 = p0.x; float y0 = p0.y;
	float x1 = p1.x; float y1 = p1.y;

	float angle = std::atan2(p1.y - p0.y, p1.x - p0.x);
	float hthic = lineThickness * 0.5;

	float hpi = PASTA_PI * 0.5f;

	float tlX = x0 + std::cos(angle + hpi) * hthic;
	float tlY = y0 + std::sin(angle + hpi) * hthic;
	float tlZ = p0.z;

	float trX = x0 + std::cos(angle - hpi) * hthic;
	float trY = y0 + std::sin(angle - hpi) * hthic;
	float trZ = p1.z;

	float blX = x1 + std::cos(angle + hpi) * hthic;
	float blY = y1 + std::sin(angle + hpi) * hthic;
	float blZ = p0.z;

	float brX = x1 + std::cos(angle - hpi) * hthic;
	float brY = y1 + std::sin(angle - hpi) * hthic;
	float brZ = p1.z;

	r::Color s = colors[0].second * gfx->geomColor;
	r::Color e = colors.back().second * gfx->geomColor;

	auto findColor = [&colors](float t0) ->r::Color {
		for (auto& p : colors) {
			if ( r::Math::essentiallyEqual(p.first,t0,0.0001f) )
				return p.second;
		}
		return r::Color::White;
	};

	auto addQuad = [=,&colors
		//,&segments
	](float t0,float t1) {
		float _tlX = Pasta::Lerp(tlX, blX, t0);
		float _tlY = Pasta::Lerp(tlY, blY, t0);
		float _tlZ = Pasta::Lerp(tlZ, blZ, t0);

		float _trX = Pasta::Lerp(trX, brX, t0);
		float _trY = Pasta::Lerp(trY, brY, t0);
		float _trZ = Pasta::Lerp(trZ, brZ, t0);

		float _blX = Pasta::Lerp(tlX, blX, t1);
		float _blY = Pasta::Lerp(tlY, blY, t1);
		float _blZ = Pasta::Lerp(tlZ, blZ, t1);
		
		float _brX = Pasta::Lerp(trX, brX, t1);
		float _brY = Pasta::Lerp(trY, brY, t1);
		float _brZ = Pasta::Lerp(trZ, brZ, t1);

		r::Color cTop = (t0 == 0.0f) ? s : findColor(t0);
		r::Color cBottom = (t1 == 1.0f) ? e : findColor(t1);

		Quad q;
		q.setP0(_tlX, _tlY, _tlZ, cTop);
		q.setP1(_trX, _trY, _trZ, cTop);
		q.setP2(_blX, _blY, _blZ, cBottom);
		q.setP3(_brX, _brY, _brZ, cBottom);
		gfx->drawQuad(q);
	};

	if (colors[0].first != 0.0) 
		addQuad(0.0, colors[0].first);

	if(colors.size()>=2)
	for (int i = 0; i < colors.size() - 1; ++i) 
		addQuad(colors[i].first, colors[i+1].first);

	if (colors[colors.size()-1].first != 1.0f) 
		addQuad(colors[colors.size() - 1].first, 1.0f);
}

void r2::ext::drawLine(r2::Graphics*gfx, const Vector3 & p0, const Vector3 & p1, const r::Color & colorStart, const r::Color & colorEnd,float thicc) {
	float lineThickness = thicc;
	float x0 = p0.x; float y0 = p0.y;
	float x1 = p1.x; float y1 = p1.y;

	float angle = std::atan2(p1.y - p0.y, p1.x - p0.x);
	float hthic = lineThickness * 0.5;

	float hpi = PASTA_PI * 0.5f;

	float tlX = x0 + std::cos(angle - hpi) * hthic;
	float tlY = y0 + std::sin(angle - hpi) * hthic;
	float blX = x0 + std::cos(angle + hpi) * hthic;
	float blY = y0 + std::sin(angle + hpi) * hthic;
	float trX = x1 + std::cos(angle - hpi) * hthic;
	float trY = y1 + std::sin(angle - hpi) * hthic;
	float brX = x1 + std::cos(angle + hpi) * hthic;
	float brY = y1 + std::sin(angle + hpi) * hthic;

	r::Color s = colorStart * gfx->geomColor;
	r::Color e = colorEnd * gfx->geomColor;

	Tri tl;
	tl.setP0(tlX, tlY, p0.z, s );
	tl.setP1(trX, trY, p1.z, e);
	tl.setP2(blX, blY, p0.z, s);

	Tri br;
	br.setP0(trX, trY, p1.z, e);
	br.setP1(brX, brY, p1.z, e);
	br.setP2(blX, blY, p0.z, s);

	gfx->drawTriangle(tl);
	gfx->drawTriangle(br);
};

void r2::ext::drawDisc(r2::Graphics* gfx, const Vector3& p, float radius, const r::Color& col, int nbSegments ) {
	if (nbSegments <= 0)	nbSegments = std::ceil(radius * 3.14 * 2 / 4);
	if (nbSegments < 3)		nbSegments = 3;

	float angle = PASTA_PI * 2 / nbSegments;
	r::Color c = col;
	for (int i = 0; i < nbSegments; i++) {
		auto a0 = i * angle;
		auto a1 = (i + 1) * angle;

		auto a0x = p.x + std::cos(a0) * radius;
		auto a0y = p.y + std::sin(a0) * radius;

		auto a1x = p.x + std::cos(a1) * radius;
		auto a1y = p.y + std::sin(a1) * radius;

		Tri t;
		t.setP0(a0x, a0y, p.z,c);
		t.setP1(a1x, a1y, p.z,c);
		t.setP2(p,c);
		gfx->drawTriangle(t);
	}
}