#pragma once

#include <string>
#include <functional>

#include <r2/RenderDoubleBuffer.hpp>

//if you want to capture a single tile filter, you can pretty much just call filter tile
namespace r2 {
	typedef std::function<void(r2::Bitmap & bmp)> BmpOp;

	enum class FilterType : u32 {
		FT_NONE,
		FT_BASE,
		FT_LAYER,
		FT_BLUR,
		FT_GLOW,
		FT_BLOOM,
		FT_COLORMATRIX,
		
		FT_COUNT,

		FT_USER = 1<<16,
	};

	enum class FilterMode : u32 {
		FM_Frozen,//single buffer the thing?
		FM_Dynamic,//double buffer the thing
	};

	class Filter {
	public:
		FilterMode				mode = FilterMode::FM_Dynamic;

		bool					enabled = true;
		bool					doRenderToBackbuffer = true;
		bool					includeScale = false; //take local scale into account for capture size, useful for filtering text
		TexFilter				texFilter = TexFilter::TF_NEAREST;
		int						nbRender = 0;//filters are double buffered by default

		std::string				name;

		RenderDoubleBuffer*		forFlattening = nullptr;
		RenderDoubleBuffer*		forFiltering = nullptr;

		Bounds					flatteningBounds;
		Node*					curNode = 0;

		FilterType				type = FilterType::FT_BASE;
								Filter();

		virtual					~Filter();

		bool					isEnabled() { return enabled; }
		void					enable(bool onOff) { enabled = onOff; }

		Bounds					getMyLocalBounds();

		std::function<void(void)> onComputeStart = {};

		//returns a perma tile that you must not delete
		//double buffered by default
		virtual Tile*			captureHierarchy(rs::GfxContext * g, Node* n, int pad = 0);

		//filters the requested tile 
		virtual Tile*			filterTile(rs::GfxContext * g, Tile* input);

		rd::Sig					beforeDraw;
		rd::Sig					afterDraw;

		/**
		* not static so we can use it in a multi pass chaining system with instance owned passes
		*/
		void					pushBitmapOp(BmpOp op);
		void					popBitmapOp();

		bool					shouldComputeFilter();
		void					compute(rs::GfxContext * g, Node* n);
		Tile*					getResult();

		void					setFrozen() { mode = FilterMode::FM_Frozen; invalidate(); };
		virtual void			invalidate();

		virtual void			im();

		virtual void			serialize(Pasta::JReflect& jr, const char* name);
		virtual Filter*			clone(r2::Filter* obj = 0);
	protected:
		bool					debugFilter = false;
		Tile*					result = nullptr;
		int						flattenPadding = 0;
		bool					useStableTextureSize = true;
		int						texPadForStability = 32;

		std::vector<BmpOp>		bmpOps;
		Bounds					filteringBounds;

		Pasta::ShadedTexture*	debugCaptureData = nullptr;
		Pasta::ShadedTexture*	debugFilterData = nullptr;
		bool					debugUseClearColor = false;
		r::Color				debugClearColor = r::Color(0.5f, 0.7f, 1, 1);

		Matrix44				cdebugProj;
		Matrix44				cdebugModel;
		Matrix44				cdebugView;

		Matrix44				fdebugProj;
		Matrix44				fdebugModel;
		Matrix44				fdebugView;
		
	};
}