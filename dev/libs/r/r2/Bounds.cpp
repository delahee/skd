#include "stdafx.h"

#include "Bounds.hpp"

#include <string>

using namespace rs;
using namespace r2;

double Bounds::left() const {
	return xMin;
}

double Bounds::right() const {
	return xMax;
}

double Bounds::down() const {
	return yMax;
}

double Bounds::top() const {
	return yMin;
}

double Bounds::up() const {
	return top();
}

double Bounds::bottom() const {
	return down();
}

std::string Bounds::toString() {
	std::string str;
	str += "tlbr(";
	str += std::to_string(left());
	str += ',';
	str += std::to_string(top());
	str += ',';
	str += std::to_string(right());
	str += ",";
	str += std::to_string(bottom());
	str += ')';
	return str;
};

int BoundsI::left() const {
	return xMin;
}

int BoundsI::right() const {
	return xMax;
}

int BoundsI::down() const {
	return yMax;
}

int BoundsI::top() const {
	return yMin;
}

int BoundsI::up() const {
	return top();
}

int BoundsI::bottom() const {
	return down();
}

bool r2::BoundsI::testCircle(int px, int py, int r) {
	int closestX = std::clamp(px, xMin, xMax);
	int closestY = std::clamp(py, yMin, yMax);

	int distX = px - closestX;
	int distY = py - closestY;

	double distSq = distX * distX + distY * distY;
	return distSq < r* r;
}

BoundsI r2::BoundsI::fromCenterSize(int x, int y, int w, int h) {
	BoundsI b;
	b.xMin = std::lrint(x - w * 0.5);
	b.yMin = std::lrint(y - w * 0.5);
	b.xMax = std::lrint(x + w * 0.5);
	b.yMax = std::lrint(y + h * 0.5);
	return b;
}

std::string BoundsI::toString() {
	std::string str;
	str += "tl br(";
	str += std::to_string(left());
	str += ",";
	str += std::to_string(top());
	str += ") , (";
	str += std::to_string(right());
	str += ",";
	str += std::to_string(bottom());
	str += ") ";

	str += std::to_string(width());
	str += "x";
	str += std::to_string(height());
	return str;
};

void BoundsI::im() {
	using namespace ImGui;
	DragInt("min", &xMin);
	DragInt("max", &xMax);
}

void r2::Bounds::im() {
	using namespace ImGui;
	DragDouble2("min", &xMin);
	DragDouble2("max", &xMax);
}