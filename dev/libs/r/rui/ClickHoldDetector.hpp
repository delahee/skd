#pragma once

#include "rd/Agent.hpp"

namespace rui {
	class Button;
	class ClickHoldDetector : public rd::Agent {
	public:
		friend				rui::Button;
		rui::Button*		bt = 0;
		bool				holding = false;
		r::proc				onHold;

							ClickHoldDetector(rui::Button* _bt);
	virtual void			update(double dt) override;
	virtual void			dispose() override;
	
	protected:
		rd::SignalHandler*  sigsHdlClick = 0;
		rd::SignalHandler*  sigsHdlBlur = 0;
		rd::SignalHandler*  sigsHdlOut = 0;
		rd::SignalHandler*  sigsHdlMDown = 0;
		rd::SignalHandler*  sigsHdlMUp = 0;
	};
}