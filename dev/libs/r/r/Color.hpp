#pragma once

#include "r/Types.hpp"

namespace r {
	struct Color {
		float r = 1.0f;
		float g = 1.0f;
		float b = 1.0f;
		float a = 1.0f;

		explicit Color(float r = 1.f,float g = 1.f, float b = 1.f,float a = 1.f) {
			this->r = r; this->g = g; this->b = b; this->a = a;
		};

		Color(const Color & col) {
			r = col.r;
			g = col.g;
			b = col.b;
			a = col.a;
		}

		//crushes rgb high 
		explicit Color(unsigned int rgb, float alpha = 1.0) {
			load(rgb & 0x00ffFFff, alpha);
		};

		//crushes rgb high 
		explicit Color(int rgb, float alpha = 1.0) {
			load(rgb & 0x00ffFFff, alpha);
		};

		explicit Color(const Pasta::Color& c) {
			r = c.r;
			g = c.g;
			b = c.b;
			a = c.a;
		};

		float* ptr() { return &r; }
		const float* ptr() const { return &r; }
		
		/**
		 * _h is [-360 ..0 ..360] 
		 * _s is in [-1 .. 0 .. 1]
		 * _v is in [-1 .. 0 ..1]
		 */
		static Color makeFromHSV(float _h, float _s_, float _v);
		void getHSV(float *_h, float *_s, float *_v) const;

		void load(int col, float alpha) {
			r = ((col >> 16) & 255) / 255.0;
			g = ((col >> 8) & 255) / 255.0;
			b = ((col) & 255) / 255.0;
			a = alpha;
		};

		void set(float r = 1.0f,
			float g = 1.0f,
			float b = 1.0f,
			float a = 1.0f) {
			this->r = r;
			this->g = g;
			this->b = b;
			this->a = a;
		};

		inline r::Color setAlpha(float a = 1.0f) {
			this->a = a;
			return *this;
		};

		static Color lerpHSV(Color a, Color b, float x);;

		static Color lerpRGB(Color a, Color b, float x) {
			return (a + ((b - a) * x));
		};

		inline r::u32 toInt() const {
			const float v0 = 0.0f;
			const float v255 = 255.0f;
			r::u32 r = (r::u32)std::clamp(this->r*v255, v0, v255);
			r::u32 g = (r::u32)std::clamp(this->g*v255, v0, v255);
			r::u32 b = (r::u32)std::clamp(this->b*v255, v0, v255);
			r::u32 a = (r::u32)std::clamp(this->a*v255, v0, v255);
			return (unsigned int)(a << 24) | (r << 16) | (g << 8) | b;
		}; 
		
		inline r::u32 toInt24() const {
			const float v0 = 0.0f;
			const float v255 = 255.0f;
			r::u32 r = (r::u32)std::clamp(this->r * v255, v0, v255);
			r::u32 g = (r::u32)std::clamp(this->g * v255, v0, v255);
			r::u32 b = (r::u32)std::clamp(this->b * v255, v0, v255);
			return (unsigned int) ((r << 16) | (g << 8) | b);
		};

		unsigned int toIntRGBA() const;

		static const Color None;
		static const Color White;
		static const Color Black;
		static const Color Grey;
		static const Color Red;
		static const Color Green;
		static const Color Blue;
		static const Color Yellow;
		static const Color Cyan;
		static const Color Magenta;
		static const Color Orange;
		static const Color Pink;
		static const Color Violet;
		static const Color Salmon;
		static const Color AcidGreen;

		operator Pasta::Color() const {
			return Pasta::Color(r, g, b, a);
		};

		Color	operator/(float k) const { return Color(r / k, g / k, b / k, a / k); }
		Color	operator * (const Color& c) const { return Color(r * c.r, g * c.g, b * c.b, a * c.a); }
		Color	operator + (const Color& c) const { return Color(r + c.r, g + c.g, b + c.b, a + c.a); }
		Color	operator - (const Color& c) const { return Color(r - c.r, g - c.g, b - c.b, a - c.a); }
		Color	operator * (float v) const { return Color(r * v, g * v, b * v, a * v); }
		bool	operator ==(const Color& other) const { return r == other.r && g == other.g && b == other.b && a == other.a; }
		bool	operator!=(const Color& v) const { return !(*this == v); }

		Color	operator=(const Color& col) {
			r = col.r;
			g = col.g;
			b = col.b;
			a = col.a;
			return col;
		};

		Color & operator=(const Pasta::Color & c) {
			r = c.r;
			g = c.g;
			b = c.b;
			a = c.a;
			return *this;
		};

		inline Color& operator*=(float f) {
			r *= f;
			g *= f;
			b *= f;
			a *= f;
			return *this;
		};
		
		inline Color scaleRGB(float f) {
			Color c(*this);
			c.r *= f;
			c.g *= f;
			c.b *= f;
			return c;
		};

		inline Color& operator*=(const Color & c) {
			r *= c.r;
			g *= c.g;
			b *= c.b;
			a *= c.a;
			return *this;
		};

		inline Color& operator+=(const Color& c) {
			r += c.r;
			g += c.g;
			b += c.b;
			a += c.a;
			return *this;
		};

		inline Color& operator-=(const Color& c) {
			r -= c.r;
			g -= c.g;
			b -= c.b;
			a -= c.a;
			return *this;
		};

		inline Color& operator/=(float fd) {
			float f = 1.0f / fd;
			r *= f;
			g *= f;
			b *= f;
			a *= f;
			return *this;
		}

		//Color operator*(const Pasta::Matrix44& mat);

		Color colorMatrix(const Pasta::Matrix44& mat);
		inline Color mulAlpha(float na) const {
			return r::Color(r, g, b, a*na);
		}
		
		inline Color mulRGB(float a) const {
			return r::Color(r * a, g * a, b * a, a);
		};

		static Color fromUIntRGBA(unsigned int col);
		static Color fromUInt(unsigned int col);
		static Color fromUInt24(unsigned int col);

		Color stringToColor(std::string col);

		Pasta::Vector4 toVec4() const {
			return Pasta::Vector4(r, g, b, a);
		};

		Color lighten(float inAmount) const {
			return Color(
				std::min(1.0f, r + inAmount),
				std::min(1.0f, g + inAmount),
				std::min(1.0f, b + inAmount),
				a
			);
		};
		
		inline r::Color saturate(float inAmount) const {
			float h, s, v;
			r::Color::getHSV(&h, &s, &v);
			s += inAmount;
			return r::Color::makeFromHSV(h, s, v);
		};

		inline float lumaFast() const {
			return (r + r + r + b + g + g + g + g) / 8.0f;
		};
		
		inline float lumaPrecise() const {
			return sqrt(0.299 * r * r + 0.587 * g * g + 0.114 * b * b);
		};

		static unsigned int toUInt(const Pasta::Vector4 & trans);

		std::string toString() const;
		std::string toHexString() const;

		

	};
}

namespace std{
	static inline std::string to_string(const Color& c) {
		return c.toString();
	};
}