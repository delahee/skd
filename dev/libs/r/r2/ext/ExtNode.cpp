#include "stdafx.h"
#include "ExtNode.hpp"

void r2::ext::texFilterNearest(r2::Node* spr) {
	spr->traverse([=](auto n) {
		auto asSpr = dynamic_cast<r2::Sprite*>(n);
		auto asTxt = dynamic_cast<r2::Text*>(n);
		if (asSpr && !asTxt)
			asSpr->texFilterNearest();
		});
}

void r2::ext::texFilterLinear(r2::Node* spr) {
	spr->traverse([=](auto n) {
		auto asSpr = dynamic_cast<r2::Sprite*>(n);
		auto asTxt = dynamic_cast<r2::Text*>(n);
		if (asSpr && !asTxt)
			asSpr->texFilterLinear();
	});
}

void r2::ext::blendAdd(r2::Node* spr) {
	spr->traverse([=](auto n) {
		auto asSpr = dynamic_cast<r2::Sprite*>(n);
		if (asSpr) asSpr->blendAdd();
	});
}
