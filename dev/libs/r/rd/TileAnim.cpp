#include "stdafx.h"
#include "TileAnim.hpp"

using namespace rd;

TileAnim::TileAnim() {
	cursor = 0;
	plays = 0;
	speed = 1.0f;
	paused = false;
	groupName = nullptr;
	groupData = nullptr;
	curFrameCount = 0;
	if (onEnd) onEnd = nullptr;
}

rd::TileAnim::TileAnim(const TileAnim& ta){
	this->cursor = ta.cursor;
	this->plays = ta.plays;
	this->speed = ta.speed;
	this->paused = ta.paused;
	this->libName = ta.libName;
	this->groupName = ta.groupName;
	this->curFrameCount = ta.curFrameCount;
	this->onEnd = ta.onEnd;
}

rd::TileAnim::~TileAnim(){
	
}

void rd::TileAnim::clear(){
	groupName = nullptr;
	groupData = nullptr;
	cursor = 0;
	curFrameCount = 0;
	plays = 0;
	paused = 0;
	speed = 1.0f;
	if (onEnd) onEnd = nullptr;
}

void TileAnim::reset(
	const char * _groupName,
	TileGroup * _data,
	int _animCursor,
	int _curFrameCpt,
	int _plays,
	bool _paused,
	float _speed
) {
	groupName = _groupName;
	groupData = _data;
	cursor = _animCursor;
	curFrameCount = _curFrameCpt;
	plays = _plays;
	paused = _paused;
	speed = _speed;
	if(onEnd) onEnd = nullptr;
}

void rd::TileAnim::copy(TileAnim & toCopy)
{
	this->cursor = toCopy.cursor;
	this->plays = toCopy.plays;
	this->speed = toCopy.speed;
	this->paused = toCopy.paused;
	this->groupName = toCopy.groupName;
	this->groupData = toCopy.groupData;
	this->curFrameCount = toCopy.curFrameCount;
	this->onEnd = toCopy.onEnd; //if returns true, cease everything for deletion is happening
}
