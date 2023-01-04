#include "stdafx.h"
#include "r2/Node.hpp"
#include "Dock.hpp"

r2::Node* rui::dock(r2::Node* node, IContainer * ref, r::DIRECTION d, float offset){
	Vector2 refFrame = ref->getRefSize();
	Vector2 frame = ref->getActualSize();
	
	if ( rd::Bits::is(d,(DOWN | UP))) 
		node->y = refFrame.y * 0.5 - node->height() * 0.5;
	else {
		if (d & DOWN)
			node->y = refFrame.y - node->height() - offset;
		if (d & UP)
			node->y = offset;
	}

	if (rd::Bits::is(d, (LEFT | RIGHT))) 
		node->x = frame.x*0.5 - node->width() * 0.5;
	else{
		if (d & RIGHT)
			node->x = frame.x - node->width() - offset;
		if (d & LEFT)
			node->x = offset;
	}
	
	return node;
}

r2::Node* rui::dock(r2::Node* node, IContainer* ref, r::DIRECTION d, Vector2 offset) {
	Vector2 refFrame = ref->getRefSize();
	Vector2 frame = ref->getActualSize();

	if (rd::Bits::is(d, (DOWN | UP)))
		node->y = refFrame.y * 0.5 - node->height() * 0.5 + offset.y;
	else {
		if (d & DOWN)
			node->y = refFrame.y - node->height() - offset.y;
		if (d & UP)
			node->y = offset.y;
	}

	if (rd::Bits::is(d, (LEFT | RIGHT)))
		node->x = frame.x * 0.5 - node->width() * 0.5 + offset.x;
	else {
		if (d & RIGHT)
			node->x = frame.x - node->width() - offset.x;
		if (d & LEFT)
			node->x = offset.x;
	}

	return node;
}
