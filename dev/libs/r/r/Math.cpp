#include "stdafx.h"

bool r::Math::isFloat(float f, float thresh) {
	return !approximatelyEqual(f, std::lrint(f), thresh);
}
