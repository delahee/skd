#pragma once

#include "Tile.hpp"
#include "Bitmap.hpp"
#include "Scene.hpp"

#include "r2/SurfacePool.hpp"

namespace r2 {
	enum class CurrentSide : u32 {
		Front,
		Back,
	};
	struct RenderDoubleBuffer {
		r2::Scene*					sc				= nullptr;
		r2::Bitmap*					bmp				= nullptr;

		bool						isFresh			= true;

		r2::TexFilter				filter			= r2::TexFilter::TF_NEAREST;
		Pasta::TextureFormat::Enum	colorFormat		= Pasta::TextureFormat::RGBA8;

									RenderDoubleBuffer(r2::TexFilter filter = r2::TexFilter::TF_NEAREST, bool isSingleBuffer = true); // force single buffer for now
									~RenderDoubleBuffer();

		void						update(int w, int h);
		void						setSingleBufferMode(bool mode);

		Pasta::Texture*				getDrawingTexture();
		Pasta::Texture*				getWorkingTexture();
		Pasta::FrameBuffer*			getDrawingFB();
		Pasta::FrameBuffer*			getWorkingFB();
		r2::Tile *					getWorkingTile();
		r2::Tile *					getDrawingTile();

		void						im();
    private:
        bool						isSingleBuffer = true;
        
		r2::Tile					workTile;
        SingleFbPage*				workBuff = nullptr;

        r2::Tile					frontTile;
        SingleFbPage*				frontBuff = nullptr;

		Pasta::ShadedTexture*		im0 = nullptr;
		Pasta::ShadedTexture*		im1 = nullptr;
		bool						imDisplayfull = false;
	};
}