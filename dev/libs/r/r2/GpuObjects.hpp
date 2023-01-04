#pragma once

#include "1-graphics/Texture.h"

namespace r2{
	class GpuObjects {
	public:
		static bool initialized;
		static Pasta::Texture *		whiteTex;
		static const r2::Tile *		whiteTile;
		static rd::Font *			defaultFont;

		static void init();
		static void dispose();

		static void onContextLost();
		static void onContextRestore();

		static Pasta::DepthStateID	depthNoSupport;
		static Pasta::DepthStateID	depthReadWrite;
		static Pasta::DepthStateID	depthReadOnly;
		static Pasta::DepthStateID	depthWriteOnly;
		static Pasta::DepthStateID	depthReadOnlyGreater;
		static Pasta::DepthStateID	depthReadEq;
		
		static Pasta::BlendStateID	opaqueNoColor;
		static Pasta::BlendStateID	alphaNoColor;
		static Pasta::BlendStateID	alphaPremulNoColor;

		//static Pasta::CoverageStateID noCoverage;
		//static Pasta::CoverageStateID defaultCoverage;
	};
}