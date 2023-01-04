#include "stdafx.h"
#include "DelayedAgent.hpp"

#define SUPER Agent
void DelayedAgent::update(double dt) {
	Agent::update(dt);

	durMs -= dt * 1000.0;

	if (durMs <= 0 && !finished) {
		if(cbk)
			cbk();

		finished = true;
		dispose();
		return;
	}
}

void DelayedAgent::dispose() {
	Agent::dispose();
}	
#undef SUPER

static rs::Pool<GDelayedAgent> gdaPool;

void GDelayedAgent::reset(){
	bus = nullptr;
}

void GDelayedAgent::update(double dt){
	if (bus)
		dt *= bus->speed;
	DelayedAgent::update(dt);
}

void GDelayedAgent::dispose() {
	bus = nullptr;
	if (pooled)
		gdaPool.free(this);
	DelayedAgent::dispose();
}

GDelayedAgent* GDelayedAgent::fromPool(std::function<void(void)> a, float b, AgentList* c ){
	if (!gdaPool.reset){
		gdaPool.reset = [](auto p) {
			p->reset();
		};
	}

	GDelayedAgent* pp = gdaPool.alloc();
	pp->durMs = b;
	pp->cbk = a;
	pp->deleteSelf = false;
	pp->pooled = true;
	if(c)c->add(pp);
	return pp;
}
