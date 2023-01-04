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

Rand& Rand::get() {
	if (_self == nullptr) _self = new Rand();
	return *_self;
}

Rand * Rand::_self = nullptr;

rd::Rand::Rand() {
	Pasta::u64 time = Pasta::Time::getTimeMarker();
	seed = (time >> 32ull) ^ time ^ 0xdeadbeef;
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
