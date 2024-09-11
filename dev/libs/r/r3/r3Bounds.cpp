#include "stdafx.h"
#include "r3/Bounds.hpp"

r2::Bounds r3::Bounds::toBounds2() const{
	r2::Bounds b;
	b.xMin = xMin;
	b.xMax = xMax;
	b.yMin = yMin;
	b.yMax = yMax;
	return b;
}

std::string r3::Bounds::toString() const{
	std::string str;
	str += "center(";
	str += std::to_string(getCenter());
	str += ", min ";
	str += std::to_string(getMin());
	str += ", max ";
	str += std::to_string(getMax());
	str += ")";
	return str;
};

void r3::Bounds::im() {
	using namespace ImGui;
	BeginGroup();
	DragDouble3("min", &xMin);
	DragDouble3("max", &xMax);

	double w = getWidth();
	double h = getHeight();
	double l = getLength();

	Value("length", l);
	Value("width", w);
	Value("height", h);
	EndGroup();
}