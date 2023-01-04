#include "stdafx.h"
#include "Cine.hpp"

void CineController::start() {
	onStart();
	cursor = 0;
	steps[cursor]->init(this);
};

void CineController::update(double dt) {
	if (cursor < 0)
		return;
	if (cursor >= steps.size())
		return;
	auto cur = steps[cursor];
	cur->update(dt);
	if (cur->isFinished()) {
		cur->dispose();
		cursor++;
		if (cursor >= steps.size())
			onEnd();
		else {
			steps[cursor]->init(this);
		}
	}
};