#pragma once

//bloat for r2::graphics goes here
#include "r/Color.hpp"
#include "r2/Graphics.hpp"

namespace r2 {
	namespace ext {
		void drawLine(r2::Graphics* gfx, const r::Vector3& p0, const r::Vector3& p1, const r::Color& colorStart, const r::Color& colorEnd, float thicc = 1.0);

		//similar to gradients but rather than gradienting linearly, uses pixel bands
		void drawBands(r2::Graphics* gfx, const r::Vector3& p0, const r::Vector3& p1, eastl::vector<eastl::pair<float, r::Color>>& colors, float thicc = 1.0);

		/**
		Colors at 0 and end will serve as padding for begin & end, if there is no 0 and 1.0 vertices, the function will insert them
		Vertices should be sorted...
		with *Unit* funcs the gradient is drawn top down in a unit square
		*/
		void drawUnitGradient(r2::Graphics* gfx, eastl::vector<eastl::pair<float,r::Color>> & colors);
		void drawGradient(r2::Graphics* gfx, const r::Vector3& p0, const r::Vector3& p1, eastl::vector<eastl::pair<float,r::Color>> & colors, float thicc = 1.0);
		void drawBoxGradient(r2::Graphics* gfx, const r::Vector3& p0, const r::Vector3& p1, eastl::vector<eastl::tuple<float,r::Color, r::Color>> & colors, float thicc = 1.0);
		void drawDisc(r2::Graphics* gfx, const r::Vector3& p0, float radius, const r::Color& col, int nbSegments = 0);
	}
}