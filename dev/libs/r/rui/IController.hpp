#pragma once

namespace rui {
	class IController {
		friend rui::Menu;
	public:
		virtual ~IController() {};
		virtual void execute() = 0;
	};
}