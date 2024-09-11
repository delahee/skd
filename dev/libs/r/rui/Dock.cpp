#include "stdafx.h"
#include "r2/Node.hpp"
#include "Dock.hpp"

using namespace rd;

r2::Node* rui::dock(r2::Node* node, IContainer * ref, rd::Dir d, float offset){
	if (!node) return node;
	if (!ref) return node;

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

	node->trsDirty = true;
	return node;
}

r2::Node* rui::dock(r2::Node* node, IContainer* ref, rd::Dir d, Vector2 offset) {
	if (!node) return node;
	Vector2 refFrame = ref->getRefSize();
	Vector2 frame = ref->getActualSize	();

	if (rd::Bits::is(d, (DOWN | UP)))
		node->y = int(refFrame.y * 0.5 - node->height() * 0.5 + offset.y);
	else {
		if (d & DOWN)
			node->y = int(refFrame.y - node->height() - offset.y);
		if (d & UP)
			node->y = offset.y;
	}

	if (rd::Bits::is(d, (LEFT | RIGHT)))
		node->x = int(frame.x * 0.5 - node->width() * 0.5 + offset.x);
	else {
		if (d & RIGHT)
			node->x = int(frame.x - node->width() - offset.x);
		if (d & LEFT)
			node->x = offset.x;
	}

	node->trsDirty = true;
	return node;
}
