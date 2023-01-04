#include "stdafx.h"
#include "r2/ext/Picking.hpp"

using namespace r;
using namespace r2;
using namespace ext;

void r2::ext::Picking::pickNode(r2::Node *ref, r2::Node * n, const r::Vector2&ev, r::u64 & res) {
	if (!n) return;
	for (r2::Node * child : n->children) {
		if (child) {
			pickNode(ref,child, ev, res);
			if (res) return;
		}
	}
	if (ref != n && !n->isUtility()) {
		Bounds b = n->getBounds(ref);
		if (b.contains(ev.x, ev.y)) {
			res = n->uid;
			return;
		}
	}
}

void r2::ext::Picking::pickNodeByType(r2::Node* ref, r2::Node* n, const r::Vector2& ev, r::u64& res, NodeType nt) {
	if (!n) return;
	for (r2::Node* child : n->children) {
		if (child) {
			pickNodeByType(ref, child, ev, res, nt);
			if (res) return;
		}
	}
	if (ref != n && n->getType() == nt ) {
		Bounds b = n->getBounds(ref);
		if (b.contains(ev.x, ev.y)) {
			res = n->uid;
			return;
		}
	}
}


void r2::ext::Picking::pickNodes(r2::Node* ref, r2::Node* n, const r::Vector2& ev, std::vector<r2::Node*>& res){
	if (!n) return;
	for (r2::Node* child : n->children) 
		if (child) 
			pickNodes(ref, child, ev, res);
	if (ref != n && !n->isUtility()) {
		Bounds b = n->getBounds(ref);
		if (b.contains(ev.x, ev.y)) 
			res.push_back( n );
	}
}

void r2::ext::Picking::pickNodeOrElem(r2::Node* ref, r2::Node* n, const r::Vector2& ev, r::u64& res, u32 rejectionMask) {
	if (!n) return;

	Batch* b = dynamic_cast<r2::Batch*>(n);
	if (b) {
		BatchElem* cur = b->head;
		Bounds b;
		while (cur) {
			
			if (rd::Bits::hasOne(cur->beFlags,rejectionMask)) {
				cur = cur->next;
				continue;
			}

			b.empty();
			cur->getBounds(b, ref);
			if (b.contains(ev.x, ev.y)) {
				res = cur->uid;
				return;
			}
			cur = cur->next;
		}
	}
	for (r2::Node* child : n->children) {
		if (child) {
			if( rd::Bits::hasOne(n->nodeFlags, rejectionMask))
				continue;

			pickNodeOrElem(ref, child, ev, res, rejectionMask);

			if (res) 
				return;
		}
	}
	if (ref != n  && !rd::Bits::hasOne(n->nodeFlags,rejectionMask)) {
		Bounds b = n->getBounds(ref);
		if (b.contains(ev.x, ev.y)) {
			res = n->uid;
			return;
		}
	}
}

void r2::ext::Picking::pickNodeOrElem(r2::Node *ref, r2::Node * n, const r::Vector2& ev, r::u64 & res){
	if (!n) return;

	Batch* b = dynamic_cast<r2::Batch*>(n);
	if (b) {
		BatchElem * cur = b->head;
		Bounds b;
		while (cur) {

			if (cur->isUtility()) {
				cur = cur->next;
				continue;
			}

			b.empty();
			cur->getBounds(b, ref);
			if (  b.contains(ev.x, ev.y)) {
				res = cur->uid;
				return;
			}
			cur = cur->next;
		}
	}
	for (r2::Node * child : n->children) {
		if (child) {
			pickNodeOrElem(ref, child, ev, res);
			if (res) return;
		}
	}
	if (ref != n && !n->isUtility()) {
		Bounds b = n->getBounds(ref);
		if (b.contains(ev.x, ev.y)) {
			res = n->uid;
			return;
		}
	}
}

void r2::ext::Picking::traverse(r2::Node* n, std::function<void(r2::Node*, r2::BatchElem*)> f) {
	if (!n) return;

	Batch* b = dynamic_cast<r2::Batch*>(n);
	if (b) {
		BatchElem* cur = b->head;
		while (cur) {
			f(nullptr, cur);
			cur = cur->next;
		}
	}

	for (r2::Node* child : n->children) 
		if (child) 
			traverse(child, f);
	f(n, nullptr);
}