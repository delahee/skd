#include "stdafx.h"
#include "Std.hpp"

std::ostream& std::operator<<(std::ostream& Str, const Pasta::Vector2& v) {
	Str << std::string("{") + std::to_string(v.x) + std::string(",") + std::to_string(v.y) + std::string("}");
	return Str;
}

std::ostream& std::operator<<(std::ostream& Str, const Pasta::Vector3i& v) {
	Str << std::string("{") + std::to_string(v.x) + std::string(",") + std::to_string(v.y) + std::string(",") + std::to_string(v.z) + std::string("}");
	return Str;
}

std::string std::to_string(const Pasta::Vector2& v) {
	return std::string("{") + std::to_string(v.x) + std::string(",") + std::to_string(v.y) + std::string("}");
}

std::string std::to_string(const Pasta::Vector3& v) {
	return std::string("{") + std::to_string(v.x) + "," + std::to_string(v.y) + "," + std::to_string(v.z) + std::string("}");
}

std::string std::to_string(const Pasta::Vector4& v) {
	return std::string("{") + std::to_string(v.x) + "," + std::to_string(v.y) + "," + std::to_string(v.z) + "," + std::to_string(v.w) + std::string("}");
}

std::string std::to_string(const Pasta::Vector3i& v) {
	return std::string("{") + std::to_string(v.x) + std::string(",") + std::to_string(v.y) + std::string(",") + std::to_string(v.z) + std::string("}");
}

std::string std::to_string(const r::Vector3s& c) {
	return std::string("{") + std::to_string(c.x) + "," + std::to_string(c.y) + "," + std::to_string(c.z) + std::string("}");
}

std::string std::to_string(const r::Matrix44& c){
	std::string s;

	for (int row = 0; row < 4; row++) {
		for (int col = 0; col < 4; col++) {
			s += to_string(c.getValue(row, col))+" ";
		}
		s.append("\n");
	}
	
	return s;
}