#pragma once

#include "Types.hpp"

namespace r{
	struct Matrix33 {
		float a  = 1.0f;
		float b  = 0.0f;
		float c  = 0.0f;
		float d  = 1.0f;
		float tx = 0.0f;
		float ty = 0.0f;

		/**
		 * Loaded with identity by default
		 */
		Matrix33(float a = 1.f, float b = 0.f, float c = 0.f, float d = 1.0f, float tx = 0.f, float ty = 0.f){
			set(a, b, c, d, tx, ty);
		}

		inline void zero() {
			a = b = c = d = tx = ty = 0.;
		}

		inline void identity() {
			a = d = 1.f;
			b = c = tx = ty = 0.;
		}

		void set(float a = 1.f, float b = 0.f, float  c = 0.f, float d = 1.f, float tx = 0.f, float  ty = 0.f) {
			this->a = a;
			this->b = b;
			this->c = c;
			this->d = d;
			this->tx = tx;
			this->ty = ty;
		}

		void invert() {
			float norm = a * d - b * c;
			if (norm == 0) {
				a = b = c = d = 0;
				tx = -tx;
				ty = -ty;
			}
			else {
				norm = 1.0 / norm;
				float a1 = d * norm;
				d = a * norm;
				a = a1;
				b *= -norm;
				c *= -norm;

				float tx1 = -a * tx - c * ty;
				ty = -b * tx - d * ty;
				tx = tx1;
			}
		}

		void rotate(float angle) {
				float c = cosf(angle);
				float s = sinf(angle);
				float z = 0.0;
				concat32(	c, s,
							-s, c,
							z,z);
		}

		void scale(float x, float y) {
			a *= x;
			c *= x;
			tx *= x;

			b *= y;
			d *= y;
			ty *= y;
		};

		void skew(float x, float y) {
			concat32(1.0, tanf(x),
				tanf(y), 1.0,
				0.0, 0.0);
		};

		inline void makeSkew(float x , float y ) {
			identity();
			b = tanf(x);
			c = tanf(y);
		}

		inline void setRotation( float angle, float scale = 1.0f) {
			a = cosf(angle) * scale;
			c = sinf(angle) * scale;
			b = -c;
			d = a;
			tx = ty = 0;
		}

		inline void setTranslation(float x, float y ) {
			identity();
			translate(x, y);
		}

		inline void setScale( float x , float y ) {
			identity();
			scale(x, y);
		}

		std::string toString();

		r::Vector2 transformPoint(const r::Vector2 & point ) {
			return Vector2(point.x * a + point.y * c + tx, point.x * b + point.y * d + ty);
		}

		void concat(const Matrix33& m);
		void concat22(const Matrix33& m);
		void concat32(float ma, float mb, float mc, float md, float mtx, float mty);;

		inline r::Vector2 transformPoint2(float pointx, float pointy, r::Vector2* res) {
			r::Vector2 tmp;
			r::Vector2& p = !res ? tmp : *res;
			auto px = pointx;
			auto py = pointy;
			p.x = px * a + py * c + tx;
			p.y = px * b + py * d + ty;
			return p;
		};

		inline float transformX(float px, float py) {
			return px * a + py * c + tx;
		};

		inline float transformY(float px, float py) {
			return px * b + py * d + ty;
		};

		inline void translate(float x, float y) {
			tx += x;
			ty += y;
		};
	};
}