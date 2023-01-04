#pragma once

#include "r/Color.hpp"

#include "1-graphics/geo_vectors.h"
#include "1-graphics/Graphic.h"
#include "1-graphics/FrameBuffer.h"

#include <vector>

namespace rs{
	enum class Pass : u32 {
		EarlyDepth = 0,
		DepthEqWrite = 1,
		Basic = 2,
		Picking = 3,
		ExtraPass0 = 4,

		PassCount,
	};

	class GfxContext {
	public:
		Pass								currentPass = Pass::Basic;
		Pasta::Graphic *					gfx = nullptr;

		//current scissor values
		int									scx = 0;
		int									scy = 0;
		int									scw = 0;
		int									sch = 0;
		Pasta::FrameBuffer *				curBuffer = nullptr;

		std::vector<Pasta::Vector4>			scissorStack;
		std::vector<Pasta::FrameBuffer*>	frameStack;//can be null for back buffer!

		static inline float					MTX_DEFAULT_ZMIN() { return -1.f; };//aka near
		static inline float					MTX_DEFAULT_ZMAX() { return  1.f; };//aka far

		float								zMin = -1.f;
		float								zMax = 1.f;

		bool								supportsEarlyDepth = false;

		//current viewport values
		s16									vpx = 0;
		s16									vpy = 0;
		u16									vpw = 0;
		u16									vph = 0;
		std::vector<Pasta::Vector4>			viewportStack;

		GfxContext();
		GfxContext(Pasta::Graphic * pastaGfx);

		~GfxContext() {
			gfx = nullptr;
		}


		void pushContext();
		void popContext();

		void pushTarget( Pasta::FrameBuffer * buff );
		void popTarget();

		void ensureConsistency();

		/**
		*does not imply push
		*/
		void setScissor(int scx, int scy, int scw, int sch);
		void pushScissor();
		//does imply a set
		void popScissor();

		/**
		*does not imply push
		*/
		void setViewport();
		void setViewport(s16 scx, s16 scy, u16 scw, u16 sch);
		void pushViewport();
		//does imply a set
		void popViewport();

		void clear(Pasta::ClearFlags _flags, const r::Color & col, float _depth = 1.f, Pasta::u8 _stencil = 0);
		void defaultZ() {
			zMin = MTX_DEFAULT_ZMIN();
			zMax = MTX_DEFAULT_ZMAX();
		};

		void								push();

		void								pop();

		void								pushViewMatrix();
		void								popViewMatrix();

		void								pushProjMatrix();
		void								popProjMatrix();
		
		void								loadViewMatrix(const r::Matrix44 & view);

		void								loadModelMatrix(const r::Matrix44 & model );
		void								loadProjMatrix(const r::Matrix44 & proj );

		void								pushZminmax();
		void								popZminmax();

		Pasta::GraphicContext*				getGpuContext();

		void								im();
		void								resetMetrics();

		std::vector<Pasta::Matrix44>		modelStack;
		std::vector<Pasta::Matrix44>		viewStack;
		std::vector<Pasta::Matrix44>		projStack;
		std::vector<Pasta::Vector2>			zminmaxStack;
		std::vector<double>					alpha;

		r::Matrix44							projMatrix;
		r::Matrix44							viewMatrix;
		r::Matrix44							modelMatrix;

		int									nbBatchElemDrawn = 0;
		int									nbDraw = 0;
	};
}
