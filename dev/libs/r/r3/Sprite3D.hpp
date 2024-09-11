#pragma once
#include "r3/Node3D.hpp"

namespace r3 {
	class Sprite3D : public Node3D {
		typedef Node3D			Super;
	public:
		rd::Vars				shaderValues;

		bool					depthRead = false;
		bool					depthWrite = false;
		bool					depthOnly = false;
		bool					hasTexcoords = false;
		bool					hasVertexColors = false;
		bool					hasNormals = false;

		r::Color				color;//color alpha is not counting against hierarchical alpha mods
		r::TransparencyType		blendmode = r::TransparencyType::TT_ALPHA;
		Pasta::ShaderProgram*	overrideShader = nullptr;
		r2::TexFilter			texFiltering = r2::TexFilter::TF_INHERIT;

	public:
								Sprite3D(Node* Parent = nullptr);
		virtual					~Sprite3D();
		virtual void			dispose() override;
		virtual void			im() override;

		virtual void			reset() override;

		virtual void			draw(rs::GfxContext* g) override;

		void					blendAlpha();
		void					blendAdd();
		void					texFilterLinear();
		void					texFilterNearest();
		void					bindTexture(rs::GfxContext* ctx, Pasta::Texture* tex, int slot);

		virtual void			setColor(int col, float a = 1.0f);
		virtual void			setColor(const r::Color& col);

		virtual double			getValue(rs::TVar valType) override;
		virtual double			setValue(rs::TVar valType, double val) override;
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