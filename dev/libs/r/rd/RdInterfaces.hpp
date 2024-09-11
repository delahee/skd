#pragma once 

#include "rd/Style.hpp"

namespace rd {
	class IStylable {
	public:
		virtual void applyStyle(const rd::Style& st) = 0;
	};
}