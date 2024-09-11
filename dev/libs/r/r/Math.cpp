#include "stdafx.h"

float r::Math::normAnglePos(float x) {
	x = fmodf(x, 2.0f * pi);
	if (x < 0)
		x += 2.0f * pi;
	return x;
}

float r::Math::normAnglePosPiNegPi(float x) {
	x = fmodf(x, 2.0f * pi);
	if (x < -pi)
		x += 2.0f * pi;
	if (x > pi)
		x -= 2.0f * pi;
	return x;
}

bool r::Math::isFloat(float f, float thresh) {
	return !approximatelyEqual(f, std::lrint(f), thresh);
}
