#include "stdafx.h"

#include "Color.hpp"

using namespace r;
using namespace std;

const Color Color::None		= Color(0,0,0,0);
const Color Color::White	= Color(1.0f, 1.0f, 1.0f, 1.0f);
const Color Color::Black	= Color(0.0f, 0.0f, 0.0f, 1.0f);
const Color Color::Grey		= Color(0.5f, 0.5f, 0.5f, 1.0f); 
const Color Color::Red		= Color(1.0f, 0.0f, 0.0f, 1.0f); 
const Color Color::Green	= Color(0.0f, 1.0f, 0.0f, 1.0f);
const Color Color::Blue		= Color(0.0f, 0.0f, 1.0f, 1.0f); 
const Color Color::Yellow	= Color(1.0f, 1.0f, 0.0f, 1.0f); 
const Color Color::Cyan		= Color(0.0f, 1.0f, 1.0f, 1.0f); 
const Color Color::Magenta	= Color(1.0f, 0.0f, 1.0f, 1.0f); 
const Color Color::Orange	= Color(1.0f, 0.5f, 0.0f, 1.0f); 
const Color Color::Pink		= Color(1.0f, 0.0f, 0.5f, 1.0f);
const Color Color::Violet	= Color(0.5f, 0.0f, 1.0f, 1.0f); 

const Color Color::Purple	= Color::fromUInt24(0x5c068c); 
const Color Color::Salmon	= Color::fromUInt24(0xFA8072);
const Color Color::AcidGreen= Color::fromUInt24(0x8ffe09);

Color Color::makeFromHSV(float _h, float _s, float _v) {
	Color RGB;
	double H = _h, S = _s, V = _v,
		P, Q, T,
		fract;

	(H == 360.) ? (H = 0.) : (H /= 60.);
	fract = H - floor(H);

	P = V * (1. - S);
	Q = V * (1. - S * fract);
	T = V * (1. - S * (1. - fract));

	if (0. <= H && H < 1.)
		RGB = Color(V, T, P);
	else if (1. <= H && H < 2.)
		RGB = Color(Q, V, P);
	else if (2. <= H && H < 3.)
		RGB = Color(P, V, T);
	else if (3. <= H && H < 4.)
		RGB = Color(P, Q, V);
	else if (4. <= H && H < 5.)
		RGB = Color(T, P, V);
	else if (5. <= H && H < 6.)
		RGB = Color(V, P, Q);
	else
		RGB = Color(0., 0., 0.);

	return RGB;
}

void Color::getHSV(float *_h, float *_s, float *_v) const {
	double      min, max, delta;

	min = r < g ? r : g;
	min = min < b ? min : b;

	max = r > g ? r : g;
	max = max > b ? max : b;

	*_v = max;                                // v
	delta = max - min;
	if (delta < 0.00001)
	{
		*_s = 0;
		*_h = 0; // undefined, maybe nan?
		return;
	}
	if (max > 0.0) { // NOTE: if Max is == 0, this divide would cause a crash
		*_s = (delta / max);                  // s
	}
	else {
		// if max is 0, then r = g = b = 0              
		// s = 0, h is undefined
		*_s = 0.0;
		*_h = NAN;                            // its now undefined
		return;
	}
	if (r >= max)                           // > is bogus, just keeps compilor happy
		*_h = (g - b) / delta;        // between yellow & magenta
	else
		if (g >= max)
			*_h = 2.0 + (b - r) / delta;  // between cyan & yellow
		else
			*_h = 4.0 + (r - g) / delta;  // between magenta & cyan

	*_h *= 60.0;                              // degrees

	if (*_h < 0.0)
		*_h += 360.0;
}

Vector3 r::Color::getHSV() const{
	Vector3 res;
	getHSV(&res.x, &res.y, &res.z);
	return res;
}

Color r::Color::lerpHSV(Color a, Color b, float x) {
	float aH=0;
	float aS=0;
	float aV=0;
	a.getHSV(&aH, &aS, &aV);
	float bH=0;
	float bS=0;
	float bV=0;
	b.getHSV(&bH, &bS, &bV);
	auto c = makeFromHSV((aH + (bH - aH) * x), (aS + (bS - aS) * x), (aV + (bV - aV) * x));
	c.a = Pasta::Lerp(a.a, b.a, x);
	return c;
}

r::u32 r::Color::toInt() const {
	const float v0 = 0.0f;
	const float v255 = 255.0f;
	r::u32 r = (r::u32)std::clamp(this->r * v255, v0, v255);
	r::u32 g = (r::u32)std::clamp(this->g * v255, v0, v255);
	r::u32 b = (r::u32)std::clamp(this->b * v255, v0, v255);
	r::u32 a = (r::u32)std::clamp(this->a * v255, v0, v255);
	return (unsigned int)(a << 24) | (r << 16) | (g << 8) | b;
}

r::u32 r::Color::toInt24() const {
	const float v0 = 0.0f;
	const float v255 = 255.0f;
	r::u32 r = (r::u32)std::clamp(this->r * v255, v0, v255);
	r::u32 g = (r::u32)std::clamp(this->g * v255, v0, v255);
	r::u32 b = (r::u32)std::clamp(this->b * v255, v0, v255);
	return (unsigned int)((r << 16) | (g << 8) | b);
}

unsigned int r::Color::toIntRGBA() const {//manage slight overflows as you can
	const float v0 = 0.0f;
	const float v255 = 255.0f;
	unsigned int r = (unsigned int)std::clamp(this->r*v255, v0, v255);
	unsigned int g = (unsigned int)std::clamp(this->g*v255, v0, v255);
	unsigned int b = (unsigned int)std::clamp(this->b*v255, v0, v255);
	unsigned int a = (unsigned int)std::clamp(this->a*v255, v0, v255);
	return (unsigned int)(a << 24) | (b << 16) | (g << 8) | r;
}


std::string r::Color::toHexString() const {
	std::stringstream sstream;
	sstream << std::hex << toInt();
	return sstream.str();
}

bool r::Color::im(const char * name){
	//produces a big thing... which eats lot of space
	//return ImGui::ColorPicker4(name, ptr(), NULL);
	return ImGui::ColorEdit4(name, ptr(), ImGuiColorEditFlags_Float | ImGuiColorEditFlags_HDR);
}

Color r::Color::fromUIntRGBA(unsigned int col) {
	Color c;
	const float inv255 = 1.0f / 255.0f;
	c.a = ((col >> 24) & 0xff)	* inv255;
	c.b = ((col >> 16) & 0xff)	* inv255;
	c.g = ((col >> 8) & 0xff)	* inv255;
	c.r = ((col) & 0xff)		* inv255;
	return c;
}

Color r::Color::fromUInt24(unsigned int col) {
	Color c;
	const float inv255 = 1.0f / 255.0f;
	c.r = ((col >> 16) & 0xff)	* inv255;
	c.g = ((col >> 8) & 0xff)	* inv255;
	c.b = ((col) & 0xff)		* inv255;
	return c;
}
Color r::Color::fromUInt(unsigned int col) {
	Color c;
	const float inv255 = 1.0f / 255.0f;
	c.a = ((col >> 24) & 0xff) * inv255;
	c.r = ((col >> 16) & 0xff) * inv255;
	c.g = ((col >> 8) & 0xff) * inv255;
	c.b = ((col) & 0xff) * inv255;
	return c;
}

unsigned int r::Color::toUInt(const Pasta::Vector4 & trans) {
	const float v0 = 0.0f;
	const float v255 = 255.0f;
	unsigned int r = (unsigned int)std::clamp<float>(trans.x*v255, v0, v255);
	unsigned int g = (unsigned int)std::clamp<float>(trans.y*v255, v0, v255);
	unsigned int b = (unsigned int)std::clamp<float>(trans.z*v255, v0, v255);
	unsigned int a = (unsigned int)std::clamp<float>(trans.w*v255, v0, v255);
	return (unsigned int)(a << 24) | (r << 16) | (g << 8) | b;
}

std::string r::Color::toString() const {
	string str;
	str += "r:" + to_string(r) + " ,";
	str += "g:" + to_string(g) + " ,";
	str += "b:" + to_string(b) + " ,";
	str += "a:" + to_string(a);
	return str;
};


Color r::Color::colorMatrix(const Matrix44& mat) {
	r::Vector4 colorVec = this->toVec4();
	r::Vector4 trans = mat * colorVec;
	return r::Color(trans.x, trans.y, trans.z, trans.w);
}

Color r::Color::stringToColor(const char * col) {
	if (!col)	return Color::White;
	if (!*col)	return Color::White;
	if (*col == '#') return rd::ColorLib::fromString(col);

			if (0 == stricmp(col, "White"))		return Color::White;
	else	if (0 == stricmp(col, "Black"))		return Color::Black;
	else	if (0 == stricmp(col, "Grey"))		return Color::Grey;
	else	if (0 == stricmp(col, "Red"))		return Color::Red;
	else	if (0 == stricmp(col, "Green"))		return Color::Green;
	else	if (0 == stricmp(col, "Blue"))		return Color::Blue;
	else	if (0 == stricmp(col, "Yellow"))	return Color::Yellow;
	else	if (0 == stricmp(col, "Cyan"))		return Color::Cyan;
	else	if (0 == stricmp(col, "Magenta"))	return Color::Magenta;
	else	if (0 == stricmp(col, "Orange"))	return Color::Orange;
	else	if (0 == stricmp(col, "Pink"))		return Color::Pink;
	else	if (0 == stricmp(col, "Violet"))	return Color::Violet;
	else	if (0 == stricmp(col, "Black"))		return Color::Black;

	return Color::White;
}

r::Color r::Color::minValue(r::Color oc, float thresh) {
	auto hsv = oc.getHSV();
	r::Color nc;
	if (hsv.z < thresh)
		nc = r::Color::makeFromHSV(hsv.x, hsv.y, hsv.z + (thresh - hsv.z));
	else
		nc = oc;
	return nc;
};

r::Color r::Color::minLum(r::Color oc, float thresh) {
	r::Color nc;
	float lum = oc.lumaPrecise();
	if (lum < thresh) 
		nc = oc.lighten(thresh - lum);
	else
		nc = oc;
	return nc;
};

void r::Color::imTest(){
	using namespace ImGui;


	static r::Color test;
	static float	prm = 1.0f;
	test.im("test");
	DragFloat("Param", &prm,0.1,-2,2);

	if (TreeNode("minValue")) {
		r::Color corrected = minValue(test, prm);
		corrected.im("fixed");
		TreePop();
	}


	if (TreeNode("minLum")) {
		r::Color corrected = minLum(test, prm);
		corrected.im("fixed");
		TreePop();
	}
}

bool r::Color::operator==(const Color& other) const { 
	return r == other.r && g == other.g && b == other.b && a == other.a; 
}

Color r::Color::operator/(float k) const { 
	if (!k)
		return Color::Black;
	float ik = 1.0f / k;
	return *this * ik;
}