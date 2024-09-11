#include "stdafx.h"
#include "Matrix33.hpp"

std::string r::Matrix33::toString() {
	return std::string() + "(a=" + std::to_string(a) + ", b=" + std::to_string(b) + ", c=" + std::to_string(c) + ", d=" + std::to_string(d)
		+ ", tx=" + std::to_string(tx) + ", ty=" + std::to_string(ty) + ")";
}

void r::Matrix33::concat(const Matrix33& m) {
	float a1 = a * m.a + b * m.c;
	b = a * m.b + b * m.d;
	a = a1;

	float c1 = c * m.a + d * m.c;
	d = c * m.b + d * m.d;

	c = c1;

	float tx1 = tx * m.a + ty * m.c + m.tx;
	ty = tx * m.b + ty * m.d + m.ty;
	tx = tx1;
}

void r::Matrix33::concat22(const Matrix33& m) {
	float a1 = a * m.a + b * m.c;
	b = a * m.b + b * m.d;
	a = a1;

	float c1 = c * m.a + d * m.c;
	d = c * m.b + d * m.d;

	c = c1;
}

void r::Matrix33::concat32(float ma, float mb, float mc, float md, float mtx, float mty) {
	float a1 = a * ma + b * mc;
	b = a * mb + b * md;
	a = a1;

	float c1 = c * ma + d * mc;
	d = c * mb + d * md;

	c = c1;

	float tx1 = tx * ma + ty * mc + mtx;
	ty = tx * mb + ty * md + mty;
	tx = tx1;
}
