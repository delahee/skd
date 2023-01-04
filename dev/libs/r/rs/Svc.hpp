#pragma once

#include <functional>
#include "rd/Agent.hpp"


namespace rs {
	class Svc {
	public:
		static bool has(rd::Agent* ag);

		//performs a has test so will never posess duplicates;
		static void reg(rd::Agent* ag, int prio=0);
		static void reg(std::function<void(double dt)> fun);
		static void reg(std::function<void(void)> fun);
		static void unreg(rd::Agent* ag);
		static void update(double dt);
		static void im();
		static int	count();
	};
}
