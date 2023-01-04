#pragma once

#include "r3/Node3D.hpp"

namespace r3 {

	class Sprite3D : public Node3D {
	public:
		rd::Vars				shaderValues;

		bool					depthRead = false;
		bool					depthWrite = false;
		bool					hasTexcoords = false;
		bool					hasNormals = false;

		//float					alpha			= 1.0f;

		r::Color				color;//color alpha is not counting against hierarchical alpha mods
		r::TransparencyType		blendmode = r::TransparencyType::TT_ALPHA;

		Pasta::ShaderProgram*	overrideShader = nullptr;
	public:
								Sprite3D(Node* Parent = nullptr);
		virtual					~Sprite3D();
		virtual void			dispose() override;
		virtual void			im() override;

		virtual void			draw(rs::GfxContext* g) override;
	protected:
		void					applyDepth(rs::GfxContext* _g);
		void					applyBlendmode(rs::GfxContext* _g);

		virtual void			bindShader(rs::GfxContext* _g);
		Pasta::ShaderProgram*	pickShader(rs::GfxContext* gfxCtx);
		/**
		* returns false if we should abort drawing
		*/
		virtual bool			drawPrepare(rs::GfxContext* _g);
		virtual void			drawSubmitGeometry(rs::GfxContext* _g) {};
		virtual void			drawCleanup(rs::GfxContext* _g);

		virtual bool			shouldRenderThisPass(rs::GfxContext* _g);
	};
}