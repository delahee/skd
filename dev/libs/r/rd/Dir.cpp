
#include "stdafx.h" 
#include "rd/Dir.hpp" 

using namespace		rd;

static const char*	 s_UP = "UP";
static const char*	 s_DOWN = "DOWN";
static const char*	 s_LEFT = "LEFT";
static const char*	 s_RIGHT = "RIGHT";
static const char*	 s_UP_LEFT = "UP_LEFT";
static const char*	 s_UP_RIGHT = "UP_RIGHT";
static const char*	 s_DOWN_LEFT = "DOWN_LEFT";
static const char*	 s_DOWN_RIGHT = "DOWN_RIGHT";

std::vector<Dir> DirLib::straightDirs = { LEFT,UP,RIGHT,DOWN };
std::vector<Dir> DirLib::allDirs = { LEFT,UP_LEFT,UP,UP_RIGHT,RIGHT, DOWN_RIGHT, DOWN,DOWN_LEFT };

const char* DirLib::dirToString(Dir dir){
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

bool DirLib::isOppositeDir(Dir a, Dir b) {
	if (a == UP && b == DOWN)
		return true;
	if (a == DOWN && b == UP)
		return true;
	if (a == LEFT && b == RIGHT)
		return true;
	if (a == RIGHT && b == LEFT)
		return true;
	return false;
}

vec2i rd::DirLib::followDir(const vec2i& v, Dir dir){
	vec2i n = v;
	if (dir & UP)
		n += vec2i(0, -1 );
	if (dir & DOWN)
		n += vec2i(0, 1 );
	if (dir & LEFT)
		n += vec2i(-1, 0 );
	if (dir & RIGHT)
		n += vec2i(1, 0 );
	return n;
}

vec2 rd::DirLib::followDir(const vec2& v, Dir dir){
	vec2 n = v;
	if (dir & UP)
		n += vec2(0, -1);
	if (dir & DOWN)
		n += vec2(0, 1);
	if (dir & LEFT)
		n += vec2(-1, 0);
	if (dir & RIGHT)
		n += vec2(1, 0);
	return n;
}

vec3i DirLib::followDir(const vec3i& v, Dir dir) {
	vec3i n = v;
	if (dir & UP)
		n += vec3i(0, -1, 0);
	if (dir & DOWN)
		n += vec3i(0, 1, 0);
	if (dir &  LEFT)
		n += vec3i(-1, 0, 0);
	if (dir &  RIGHT)
		n += vec3i(1, 0, 0);
	return n;
}
