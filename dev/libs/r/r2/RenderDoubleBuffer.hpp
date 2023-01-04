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
		r2::Tile					workingTile;
		r2::Tile					backingTile;

		SingleFbPage*				workBuff		= nullptr;
		SingleFbPage*				backingBuff		= nullptr;

		r2::Scene*					sc				= nullptr;
		r2::Bitmap*					bmp				= nullptr;

		bool						isSingleBuffer	= false;
		bool						isFresh			= true;

		r2::TexFilter				filter			= r2::TexFilter::TF_NEAREST;
		Pasta::TextureFormat::Enum	colorFormat		= Pasta::TextureFormat::RGBA8;

		bool						swapped			= false;
		CurrentSide					side			= CurrentSide::Front;

		RenderDoubleBuffer(r2::TexFilter filter = r2::TexFilter::TF_NEAREST);
		~RenderDoubleBuffer();

		//returns true if surface was resized
		void				update(int w, int h);

		//returns the drawing tile and swaps
		//r2::Tile *			swapBuffer();
		Pasta::Texture*		getDrawingTexture();
		Pasta::Texture*		getWorkingTexture();
		Pasta::FrameBuffer* getDrawingFB();
		Pasta::FrameBuffer*	getWorkingFB(); 

		r2::Tile *			getWorkingTile();
		r2::Tile *			getDrawingTile(); 

		void				im();

		Pasta::ShadedTexture*	im0 = nullptr;
		Pasta::ShadedTexture*	im1 = nullptr;
		bool					imDisplayfull = false;
		
	};
}