#include "stdafx.h"
#include "r/Color.hpp"
#include "ColorLib.hpp"

using namespace std;
using namespace r;
using namespace rd;

r::Color rd::ColorLib::getHSV(r::Color c){
	float K = 0.f;
	if (c.g < c.b)
	{
		std::swap(c.g, c.b);
		K = -1.f;
	}

	if (c.r < c.g)
	{
		std::swap(c.r, c.g);
		K = -2.f / 6.f - K;
	}

	r::Color res;
	const float chroma = c.r - (c.g < c.b ? c.g : c.b);
	res.r = std::fabsf(K + (c.g - c.b) / (6.f * chroma + 1e-20f));
	res.g = chroma / (c.r + 1e-20f);
	res.b = c.r;
	return res;
}

r::Color rd::ColorLib::fromHSV(r::Color hsv) {
	float s = hsv.g;
	float v = hsv.b;
	r::Color out;

	if (s == 0.0f)
	{
		// gray
		out.r = out.g = out.b = v;
		return out;
	}

	float h = std::fmodf(hsv.r, 1.0f) / (60.0f / 360.0f);
	int   i = (int)h;
	float f = h - (float)i;
	float p = v * (1.0f - s);
	float q = v * (1.0f - s * f);
	float t = v * (1.0f - s * (1.0f - f));

	switch (i)
	{
		case 0: out.r = v; out.g = t; out.b = p; break;
		case 1: out.r = q; out.g = v; out.b = p; break;
		case 2: out.r = p; out.g = v; out.b = t; break;
		case 3: out.r = p; out.g = q; out.b = v; break;
		case 4: out.r = t; out.g = p; out.b = v; break;
		case 5: default: out.r = v; out.g = p; out.b = q; break;
	}
	return out;
}

r::Color rd::ColorLib::fromInt(unsigned int argb)
{
	const float one255 = 1.0f / 255.0f;

	float r = (argb >> 16	) & 0xff;
	float g = (argb >> 8	) & 0xff;
	float b = (argb			) & 0xff;
	float a = (argb >> 24	) & 0xff;

	r *= one255;
	g *= one255;
	b *= one255;
	a *= one255;

	return r::Color( r,g,b,a);
}

r::Color rd::ColorLib::fromString(const char * val) {
	if (!val || !*val) return r::Color::White;

	if (strchr(val, '#')) {
		val++;
		r::Color v = fromInt(strtoul(val, nullptr, 16));
		if (strlen(val) == 6) 
			v.a = 1.0f;
		return v;
	}
	if (strstr(val, "0x")) {
		val += 2;
		r::Color v = fromInt(strtoul(val, nullptr, 16));
		if (strlen(val) == 6) v.a = 1.0f;
		return v;
	}

	int v = atoi(val);
	if (v == 0) 
		return r::Color::stringToColor(val);
	return fromInt(v);
}

r::Color rd::ColorLib::fromString(const string & val){
	return fromString(val.c_str());
}

r::Color rd::ColorLib::get(unsigned int rgb, float alpha){
	r::Color lrgb = fromInt(rgb&0x00ffffff);
	lrgb.a = alpha;
	return lrgb;
}

//todo check
r::Color rd::ColorLib::getGreyscale(r::Color c)
{
	//0.2126 R + 0.7152 G + 0.0722 B
	return r::Color(0.2126 * c.r, 0.7152 * c.g, 0.0722 * c.b);
}

//todo check
r::Color rd::ColorLib::saturate(r::Color c, float delta)
{
	r::Color hsv = getHSV(c);
	hsv.g += delta;
	if (hsv.g>1) hsv.g = 1;
	if (hsv.g<0) hsv.g = 0;
	return hsv;
}

unsigned int rd::ColorLib::toInt(const string & c) {
	if (strchr(c.c_str(), '#'))		return strtoul(c.c_str() + 1, nullptr, 16);
	if (strstr(c.c_str(), "0x"))		return strtoul(c.c_str() + 2, nullptr, 16);
	return atoi(c.c_str());
}

unsigned int rd::ColorLib::toInt(r::Color c) {
	int r = c.r * 255.0;
	int g = c.g * 255.0;
	int b = c.b * 255.0;
	int a = c.a * 255.0;

	return (a << 24) | (r << 16) | (g << 8) | b;
}

// ---- COLOR MATRIX FUNCTIONS -------

static float lumR = 0.212671f;
static float lumG = 0.71516f;
static float lumB = 0.072169f;


/**
* identity is zero
*/
Matrix44 & ColorLib::colorContrast(Pasta::Matrix44 & mat, float contrast ) {
	Matrix44 tmp;
	float v = contrast + 1;

	tmp.getValue(0, 0) = v;
	tmp.getValue(0, 1) = 0;
	tmp.getValue(0, 2) = 0;

	tmp.getValue(1, 0) = 0;
	tmp.getValue(1, 1) = v;
	tmp.getValue(1, 2) = 0;

	tmp.getValue(2, 0) = 0;
	tmp.getValue(2, 1) = 0;
	tmp.getValue(2, 2) = v;

	tmp.getValue(3, 0) = -contrast*0.5;
	tmp.getValue(3, 1) = -contrast*0.5;
	tmp.getValue(3, 2) = -contrast*0.5;

	Matrix43 tmp43(tmp);
	mat = mat * tmp43;

	return mat;
}

Matrix44 & ColorLib::colorBrightness(Pasta::Matrix44 & mat, float brightness ) {
	mat.getValue(0, 3 ) += brightness;
	mat.getValue(1, 3 ) += brightness;
	mat.getValue(2, 3 ) += brightness;
	return mat;
}

Matrix44 & ColorLib::colorSaturation(Pasta::Matrix44 & mat, float sat ) {
	float is = 1 - sat;
	float r = is * lumR;
	float g = is * lumG;
	float b = is * lumB;

	Matrix44 tmp;
	tmp.getValue(0, 0) = r + sat;
	tmp.getValue(0, 1) = r;
	tmp.getValue(0, 2) = r;

	tmp.getValue(1, 0) = g;
	tmp.getValue(1, 1) = g + sat;
	tmp.getValue(1, 2) = g;

	tmp.getValue(2, 0) = b;
	tmp.getValue(2, 1) = b;
	tmp.getValue(2, 2) = b + sat;

	tmp.getValue(3, 0) = 0;
	tmp.getValue(3, 1) = 0;
	tmp.getValue(3, 2) = 0;

	Matrix43 tmp43(tmp);
	mat = mat * tmp43; 

	return mat;
}

static double SQ13 = 0.57735026918962576450914878050196; // sqrt(1/3)
Matrix44 & ColorLib::colorHue(Pasta::Matrix44 & mat, float hue ) {
	if (hue == 0.) return mat;
	float cosA = cos(hue);
	float sinA = sin(hue);
	float ch = (1.0f - cosA) / 3.0f;

	//meh, pas ouf comme rendu mais mieux qu'avant deja, repris de heaps
	Matrix44 tmp;
	tmp.getValue(0, 0) = cosA + ch;
	tmp.getValue(0, 1) = ch - SQ13 * sinA;
	tmp.getValue(0, 2) = ch + SQ13 * sinA;

	tmp.getValue(1, 0) = ch + SQ13 * sinA;
	tmp.getValue(1, 1) = cosA + ch;
	tmp.getValue(1, 2) = ch - SQ13 * sinA;

	tmp.getValue(2, 0) = ch - SQ13 * sinA;
	tmp.getValue(2, 1) = ch + SQ13 * sinA;
	tmp.getValue(2, 2) = cosA + ch;

	tmp.getValue(3, 0) = 0;
	tmp.getValue(3, 1) = 0;
	tmp.getValue(3, 2) = 0;
	tmp.getValue(3, 3) = 0;

	Matrix43 tmp43(tmp);
	mat = mat * tmp43;

	return mat;
}

Matrix44 & rd::ColorLib::colorHSV(Pasta::Matrix44 & mat, float hue, float sat, float v) {
	float vsu = v * sat * cos(hue * PASTA_PI / 180);
	float vsw = v * sat * sin(hue * PASTA_PI / 180);

	// cf http://beesbuzz.biz/code/16-hsv-color-transforms
	// not perfect but better than before
	mat.setRow(0, Pasta::Vector3(
		.299 * v + .701 * vsu + .168 * vsw,
		.299 * v - .299 * vsu - .328 * vsw,
		.299 * v - .300 * vsu + 1.25 * vsw
	));
	mat.setRow(1, Pasta::Vector3(
		.587 * v - .587 * vsu + .330 * vsw,
		.587 * v + .413 * vsu + .035 * vsw,
		.587 * v - .588 * vsu - 1.05 * vsw
	));
	mat.setRow(2, Pasta::Vector3(
		.114 * v - .114 * vsu - .497 * vsw,
		.114 * v - .114 * vsu + .292 * vsw,
		.114 * v + .886 * vsu - .203 * vsw
	));

	return mat;
}

Matrix44 & rd::ColorLib::colorColorize(Pasta::Matrix44 & mat, const r::Color & rgb, float ratioNewColor, float ratioOldColor, float alpha) {
	float r = ratioNewColor * rgb.r;
	float g = ratioNewColor * rgb.g;
	float b = ratioNewColor * rgb.b;

	Matrix44 tmp;

	tmp.setRow(0, Vector3(ratioOldColor + r, g, b));
	tmp.setRow(1, Vector3(r, ratioOldColor + g, b));
	tmp.setRow(2, Vector3(r, g, ratioOldColor + b));
	tmp.setRow(3, Vector4(0, 0, 0, alpha));

	mat = mat * tmp;
	return mat;
}

Matrix44 & rd::ColorLib::colorColorizeInt(Pasta::Matrix44 & mat, unsigned int col, float ratioNewColor, float ratioOldColor ,float alpha){
	return colorColorize(mat, ColorLib::fromInt(col), ratioNewColor, ratioOldColor, alpha);
}
