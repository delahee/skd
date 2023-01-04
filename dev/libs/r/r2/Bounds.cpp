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