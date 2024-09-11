#include "stdafx.h"
#include "DelayedAgent.hpp"

using namespace rd;
DelayedAgent::DelayedAgent(std::function<void(void)> _cbk, float delayMs, AgentList* list) : Agent(list) {
	setName("DelayedAgent");
	cbk = _cbk;
	durMs = delayMs;
}

DelayedAgent::DelayedAgent() : Agent(0,0) {
	setName("DelayedAgent");
}

DelayedAgent::~DelayedAgent() {

	dispose();
}

void DelayedAgent::onDispose() {
	cbk = {};
}

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


static rs::Pool<GDelayedAgent> gdaPool;

void GDelayedAgent::reset(){
	bus = nullptr;
}

void GDelayedAgent::update(double dt){
	if (bus)
		dt *= bus->speed;
	DelayedAgent::update(dt);
}

void GDelayedAgent::onDispose() {
	bus = nullptr;
}

void GDelayedAgent::dispose() {
	onDispose();
	detach();
	if (pooled)
		gdaPool.release(this);
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
