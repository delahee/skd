#include "stdafx.h"
#include "Types.hpp"

using namespace r;

static const char* s_UP = "UP";
static const char* s_DOWN = "DOWN";
static const char* s_LEFT = "LEFT";
static const char* s_RIGHT = "RIGHT";
static const char* s_UP_LEFT = "UP_LEFT";
static const char* s_UP_RIGHT = "UP_RIGHT";
static const char* s_DOWN_LEFT = "DOWN_LEFT";
static const char* s_DOWN_RIGHT = "DOWN_RIGHT";

std::vector<DIRECTION> r::straightDirs = { LEFT,UP,RIGHT,DOWN };

std::vector<DIRECTION> r::allDirs = { LEFT,UP_LEFT,UP,UP_RIGHT,RIGHT, DOWN_RIGHT, DOWN,DOWN_LEFT };

const char* r::dirToString(r::DIRECTION dir)
{
	switch (dir)
	{
	case UP:		return s_UP;
	case DOWN:		return s_DOWN;
	case LEFT:		return s_LEFT;
	case RIGHT:		return s_RIGHT;
	case UP_LEFT:	return s_UP_LEFT;
	case UP_RIGHT:	return s_UP_RIGHT;
	case DOWN_LEFT:	return s_DOWN_LEFT;
	case DOWN_RIGHT:return s_DOWN_RIGHT;
	default:
		return s_UP;
	}
}

Vector3i r::followDir(const Vector3i& v, DIRECTION dir){
	Vector3i n = v;
	if( dir & r::UP )
		n += Vector3i(0, -1, 0);
	if (dir & r::DOWN)
		n += Vector3i(0, 1, 0);
	if (dir & r::LEFT)
		n +=  Vector3i(-1, 0, 0);
	if (dir & r::RIGHT)
		n += Vector3i(1, 0, 0);
	return n;
}
