#pragma once

#include "EASTL/vector.h"
#include "1-graphics/Texture.h"
#include "1-graphics/FrameBuffer.h"
#include "r2/Types.hpp"

namespace r2 {
	class Page {
	public:
		int64_t						dirtyTime = 0;//set to rs::Timer::FrameCount when used

		int							w = 0;
		int							h = 0;
		r2::TexFilter				filter = r2::TexFilter::TF_NEAREST;
		Pasta::TextureFormat::Enum	colorFormat = Pasta::TextureFormat::RGBA8;

		Page() {};
		virtual void	swapBuffer() {};
		virtual ~Page() { };
	};

	//grants double buffered maanged framebuffers, beware
	struct DoubleFbPage : public Page {
	public:
		DoubleFbPage() {};
		Pasta::FrameBuffer *	frontBuffer = nullptr;
		Pasta::FrameBuffer *	backBuffer = nullptr;

		Pasta::Texture*			getRt();
		virtual void			swapBuffer();
		virtual					~DoubleFbPage();
	};

	struct SingleFbPage : public Page {
	public:
		Pasta::FrameBuffer*		buffer = nullptr;
								SingleFbPage() {};
		Pasta::Texture*			getRt();
		virtual					~SingleFbPage();
	};

	class SurfacePool {
	private:
		SurfacePool();
		~SurfacePool();

	public:
		bool	enabled			= true;
		static	SurfacePool*	get();

		virtual DoubleFbPage *	allocDoubleFb(int w, int h, r2::TexFilter filter );
		virtual SingleFbPage *	allocSingleFb(int w, int h, r2::TexFilter filter, bool depth = true, Pasta::TextureFormat::Enum colorFormat = Pasta::TextureFormat::RGBA8);
		virtual	void			freeDfb(DoubleFbPage*);
		virtual	void			freeSfb(SingleFbPage*);
		void					update();
		void					im();
	
		uint64_t				curTime = 0;
		int						DELETE_SURFACE_AGE = 10;
	protected:
		eastl::vector<DoubleFbPage*>	dfbs;
		eastl::vector<SingleFbPage*>	sfbs;
	};
}