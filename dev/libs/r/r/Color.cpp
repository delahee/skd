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

unsigned int r::Color::toIntRGBA() const {
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
};

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

Color r::Color::stringToColor(string col) {

	if		(col == "White")	return Color::White;
	else if (col == "Black")	return Color::Black;
	else if (col == "Grey")		return Color::Grey;
	else if (col == "Red")		return Color::Red;
	else if (col == "Green")	return Color::Green;
	else if (col == "Blue")		return Color::Blue;
	else if (col == "Yellow")	return Color::Yellow;
	else if (col == "Cyan")		return Color::Cyan;
	else if (col == "Magenta")	return Color::Magenta;
	else if (col == "Orange")	return Color::Orange;
	else if (col == "Pink")		return Color::Pink;
	else if (col == "Violet")	return Color::Violet;
	else						return Color::Black;

}
