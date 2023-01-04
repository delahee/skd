#pragma once 

#include "1-graphics/Texture.h"
#include "r2/Types.hpp"
#include "r2/RenderDoubleBuffer.hpp"
#include "r2/Tile.hpp"

namespace r2 {
	namespace svc {
		/*
		* Capture element with possible padding
		* if pad is specified, the resulting tile will incorporate the padding
		*/
		class Capture {
		public:
			//if you want to keep your own buffers
			Capture();


			//all tiles are served back with 0,0 and w+pad*2 h+pad*2 help yourself with that...
			//returned tile is meant for drawing
			//for reworking or refiltering, use getWorkingT
			r2::Tile* make(r::Texture * src, const std::vector<BmpOp> & bmpOps, r2::TexFilter filter = r2::TexFilter::TF_NEAREST, int pad = 0);
			r2::Tile* make(r2::Tile * src, const std::vector<BmpOp> & bmpOps, r2::TexFilter filter = r2::TexFilter::TF_NEAREST, int pad = 0);

			//tile is served back with original dx dy and pad taken into account
			//returned tile is meant for drawing
			//for reworking or refiltering, use getWorkingTile
			r2::Tile* make(r2::Node * src, r2::TexFilter filter = r2::TexFilter::TF_NEAREST, int pad = 0);

			RenderDoubleBuffer				rd;
			r2::Tile						dummy;
			r2::Tile						stub;

			int								additionnalPad = 0;
			int								texPadForStability = 64;
			bool							useStableTextureSize = false;
			bool							isSingleBuffer = false;

			void							im();

			static bool						recordTile;
			static eastl::vector<r2::Tile>	debugTiles;
			Pasta::ShadedTexture*			debugCaptureData = nullptr;
			bool							debugClearColor = false;
			r::Color						debugClearColorVal;
			Bounds							debugBounds;

			r2::Tile*						getWorkingTile() { return rd.getWorkingTile(); };
		};
	}
}