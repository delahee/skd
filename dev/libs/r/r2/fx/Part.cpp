#include "stdafx.h"

#include "Part.hpp"

using namespace r2;
using namespace r2::fx;
Part::Part(r2::Sprite * sp, rd::AgentList * al) : rd::Agent(al) {
	x = sp->x;
	y = sp->y;
	rd::Agent::deleteSelf = true;
}

float Part::setDelay(float d){
	spr->visible = d <= 0;
	return delay = d;
}

float Part::setLife(float l) {
	if (l<0)
		l = 0;
	rLife = l;
	maxLife = l;
	return l;
}

void Part::update(double dt){
	double dfr = dt * rs::Sys::FPS;

	setDelay(delay - dfr);

	if (delay > 0.f || killed) {
		syncPos();
		return;
	}

	if (onStart != nullptr) {
		auto cb = onStart;
		onStart = nullptr;
		cb(*this);
	}

	dx += gx * dfr;
	dy += gy * dfr;
	
	dx *= pow(frictX, dfr);
	dy *= pow(frictY, dfr);

	x += dx * dfr;
	y += dy * dfr;
	
	// Ground
	if (useGround && (dy>0) && (y >= groundY) ) {
		dy = -dy*bounceMul;
		y = groundY - 1;
		if (onBounce) onBounce(*this);
	}
	
	spr->rotation += dr * dfr;

	float tdsx = ds + dsx;
	float tdsy = ds + dsy;

	spr->scaleX += tdsx * dfr;
	spr->scaleY += tdsy * dfr;

	spr->scaleX *= pow(scaleMul, dfr);
	spr->scaleY *= pow(scaleMul, dfr);

	// Fade in
	if (rLife > 0 && da != 0) {
		alpha += da * dfr;
		if (alpha>maxAlpha) {
			da = 0;
			alpha = maxAlpha;
		}
	}
	
	rLife -= dfr;

	// Fade out (life)
	if (rLife <= 0)
		alpha -= fadeOutSpeed;

	// Death
	if (rLife <= 0
		&&	(alpha <= 0 || killOnLifeOut || (useBounds && !bounds.contains(x, y)))) {
		if (onKill) {
			auto cb = onKill;
			onKill = nullptr;
			cb(*this);
		}
		kill();
	}
	else if (onUpdate)
		onUpdate(*this);

	fr+=dfr;

	syncPos();
}

void Part::syncPos() {
	spr->x = x;
	spr->y = y;
}

void Part::kill(){
	alpha = 0;
	rLife = 0;
	killed = true;

	dispose();
}

