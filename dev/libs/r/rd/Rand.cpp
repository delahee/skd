#include "stdafx.h"
#include "Rand.hpp"

using namespace rd;

int rd::Rand::temper() {
	unsigned int t0 = 0;
	unsigned int t1 = 0;

	t0 = status[3];
	t1 = status[0] + (status[2] >> tinySh8);

	t0 ^= t1;

	int t1s = t1 & 1;
	t0 ^= (-t1s) & tmat;
	return t0;
}

void rd::Rand::im(const char * prefix){
	using namespace ImGui;
#ifdef _DEBUG
	bool opened = false;
	if (prefix) {
		opened = TreeNode(std::string(prefix) + " " + std::to_string(seed));
		if (!opened)
			return;
	}

	DragInt("seed", &seed);
	LabelText("status","0x%x 0x%x 0x%x 0x%x", status[0], status[1], status[2], status[3]);
	if (Button(ICON_MD_CAKE)) {
		*this = {};
	}
	if (prefix && opened)
		 TreePop();
#endif
}

std::string rd::Rand::toString(){
#ifndef _DEBUG
	return {};
#else
	std::string  res;
	res += std::to_string(seed);
	return res;
#endif
}

Rand& Rand::get() {
	if (_self == nullptr) _self = new Rand();
	return *_self;
}

Rand * Rand::_self = nullptr;

rd::Rand::Rand() {
	Pasta::u64 time = Pasta::Time::getTimeMarker();
	int seed = (time >> 32ull) ^ time ^ 0xdeadbeef;
	init(seed);
}

rd::Rand::Rand(int seed){
	init(seed);
}

void rd::Rand::init(int seed){
	status[0] = seed;
	status[1] = mat1;
	status[2] = mat2;
	status[3] = tmat;
	for (int i = 0; i < 8; ++i) {
		status[i & 3] ^= i + 1812433253
			* (status[(i - 1) & 3]
				^ (status[(i - 1) & 3] >> 30));
	}

#ifdef _DEBUG
	this->seed = seed;
#endif

	for (int i = 0; i < 8; ++i)
		nextState();
}

void rd::Rand::nextState(){
	unsigned int x = 0;
	unsigned int y = 0;

	y = status[3];
	x = (status[0] & tinyMask)
		^ status[1]
		^ status[2];

	x ^= (x << tinySh0);
	y ^= (y >> tinySh1) ^ x;

	status[0] = status[1];
	status[1] = status[2];

	status[2] = x ^ (y << tinySh1);
	status[3] = y;

	int ly = y & 1;
	status[1] ^= -ly & mat1;
	status[2] ^= -ly & mat2;
}


Vector2 rd::Rand::circle(float _radius){
	float a = angle();
	return Vector2( cosf(a * _radius),sinf(a * _radius));
}