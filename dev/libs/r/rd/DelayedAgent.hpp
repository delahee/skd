#pragma once

#include "Agent.hpp"

using namespace rd;

class DelayedAgent : public Agent {
public:
	float durMs = 0.f;
	std::function<void(void)> cbk;
	bool finished = false;

	DelayedAgent(std::function<void(void)> _cbk, float delayMs, AgentList * list = nullptr) : Agent(list) {
		cbk = _cbk;
		durMs = delayMs;
		deleteSelf = true;
	}

	DelayedAgent() : Agent(nullptr) {}

	virtual ~DelayedAgent() {}
	virtual void update(double dt);
	virtual void dispose() override;
};

class GDelayedAgent : public DelayedAgent {
public:
	AnimBus*	bus = nullptr;
	bool		pooled = false;

	GDelayedAgent(std::function<void(void)> a, float b, AgentList* c = nullptr) : DelayedAgent(a,b,c) {
		
	}

	GDelayedAgent() : DelayedAgent() {
	}

	virtual ~GDelayedAgent(){}

	void					reset();
	virtual void			update(double dt) override;
	virtual void			dispose() override;

	static GDelayedAgent*	fromPool(std::function<void(void)> a, float b, AgentList* c = nullptr);
};