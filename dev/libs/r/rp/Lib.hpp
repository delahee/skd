#pragma once

#include "r2/Graphics.hpp"

/**
* rp is the prototyping namespace
* also serves as physics but who gives a care...
* func names should be as short as possible and args expressive and automatics
*/
namespace rp {
	inline r2::Graphics * line(float x, float y, float xx, float yy, float thicc, r::Color c = r::Color::White, r2::Node * parent = nullptr ) {
		r2::Graphics * g = new r2::Graphics(parent);
		g->geomColor = c;
		g->drawLine(x, y, xx, yy,thicc);
		return g;
	}

	inline r::Color	palPepsPink		= r::Color::fromUInt24(0xF279B2);
	inline r::Color	palPepsGreen	= r::Color::fromUInt24(0x05F2C7);
	inline r::Color	palPepsYellow	= r::Color::fromUInt24(0xF2D16D);
	inline r::Color	palPepsRed		= r::Color::fromUInt24(0xF24C27);
	inline r::Color	palPepsPomelos	= r::Color::fromUInt24(0xF2836B);
}