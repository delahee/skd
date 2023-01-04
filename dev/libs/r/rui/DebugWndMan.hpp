#pragma once

#include "rd/Agent.hpp"

namespace rui {
	class DebugWndMan : public rd::Agent {
	public:
		DebugWndMan(rd::AgentList* al = 0);
		virtual void update(double dt) override;

		int						cur = -1;
		std::vector<Vector2i>	resos;
	};
}