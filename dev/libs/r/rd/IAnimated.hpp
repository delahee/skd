#pragma once

#include "TileLib.hpp"

namespace rd{
	class TileLib;

	class IAnimated {
	public:
		virtual void setFrame(int frame)	= 0;
		virtual int getFrame()				= 0;
		virtual void dispose()				= 0;
		virtual TileLib * getLib()			= 0;
		virtual void set(TileLib * l = nullptr, const char* group = nullptr, int frame = 0, bool stopAllAnims = false) = 0;
	};
}