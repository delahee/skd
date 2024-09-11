#pragma once

#include "Agent.hpp"

namespace rd {
	class DelayedAgent : public Agent {
		typedef DelayedAgent Super;
	public:
		float durMs = 0.f;
		std::function<void(void)> cbk;
		bool finished = false;

		DelayedAgent(std::function<void(void)> _cbk, float delayMs, AgentList* list = nullptr);
		DelayedAgent();

		virtual ~DelayedAgent();
		virtual void onDispose()override;
		virtual void update(double dt);
	};

	class GDelayedAgent : public DelayedAgent {
		typedef DelayedAgent Super;

	public:
		AnimBus* bus = nullptr;
		bool		pooled = false;

		GDelayedAgent(std::function<void(void)> a, float b, AgentList* c = nullptr) : DelayedAgent(a, b, c) {

		}

		GDelayedAgent() : DelayedAgent() {
		}

		virtual ~GDelayedAgent() {}

		void					reset();
		virtual void			update(double dt) override;
		virtual	void			onDispose() override;
		virtual void			dispose() override;

		static GDelayedAgent* fromPool(std::function<void(void)> a, float b, AgentList* c = nullptr);
	};
}