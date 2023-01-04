#pragma once

#include "r2/Types.hpp"
#include "r2/Node.hpp"

namespace r2{
	class Filter;

	class Sprite : public r2::Node {
	
	public:
		bool					depthRead				= false;
		bool					depthWrite				= false;
		bool					depthGreater			= false;
		bool					forceBasicPass			= false;
		bool					useExtraPass			= false;
		bool					killAlpha				= false;
		bool					useSimpleZ				= true;

		TexFilter				texFiltering	= r2::TexFilter::TF_INHERIT;

		float					zTopOffset		= 0.0f;
		float					zBottomOffset	= 0.0f;

		r::Color				color;//color alpha is not counting against hierarchical alpha mods
		r::TransparencyType		blendmode = r::TransparencyType::TT_ALPHA;
		
	public:
		double					vcustom0 = 0.0, vcustom1 = 0.0, vcustom2 = 0.0, vcustom3 = 0.0,
								vcustom4 = 0.0, vcustom5 = 0.0, vcustom6 = 0.0, vcustom7 = 0.0;

		Pasta::ShaderProgram*					overrideShader = nullptr;
		Pasta::ShaderProgram*					overrideEarlyDepthShader = nullptr;
		r::Texture*								additionnalTexture = nullptr;
		std::function<void(r2::Sprite*)>		beforeExtraPass;
		std::function<void(r2::Sprite*)>		afterExtraPass;

	public:
								Sprite( Node * Parent = nullptr);
		virtual					~Sprite();

		virtual void			dispose() override;

		virtual	void			reset();

		virtual void			draw(rs::GfxContext* ctx) override;

		virtual void			setColor(int col, float a = 1.0f);
		virtual void			setColor(const r::Color &col);

		virtual void			setZOffset(float topOffset, float bottomOffset);

		virtual Pasta::Texture * getTexture(rs::GfxContext* ctx = nullptr, int slot = 0);

		r2::Shader				shader				= Shader::SH_Basic;
		unsigned int			shaderFlags			= 0;

		//deprecated for runtime std::string construction allocates
		void					setShaderParam(const std::string & name, int val);

		//deprecated for runtime std::string construction allocates
		void					setShaderParam(const std::string& name, float val);

		//deprecated for runtime std::string construction allocates
		void					setShaderParam(const std::string& name, const float* buf, int nbFloats);

		void					setShaderParam(const char * name, int val);
		void					setShaderParam(const char * name, float val);
		void					setShaderParam(const char * name, const float* buf, int nbFloats);

		//deprecated for runtime std::string construction allocates
		void					updateShaderParam(const std::string & name, float val);

		//deprecated for runtime std::string construction allocates
		void					updateShaderParam(const std::string& name, const float* buf, int nbFloats);

		//deprecated for runtime std::string construction allocates
		void					updateShaderParam(const std::string& name, rd::Anon* val);

		void					updateShaderParam(const char * name, float val);
		void					updateShaderParam(const char* name, const float* buf, int nbFloats);
		void					updateShaderParam(const char* name, rd::Anon* val);

		bool					hasShaderParam(const char * name);
		bool					hasShaderParam(const std::string & name);

		void					removeShaderParam(const char *  name);
		void					removeShaderParam(const std::string & name);

		void					addShaderParam(rd::Anon * val);
		

		void					mkClassic();
		void					mkUber();

		rd::Anon*				getShaderParam(const char * str);
		rd::Anon*				getShaderParam(const std::string & name);

		virtual Tile *			getPrimaryTile();
		Tile *					getRenderTile();

		void					applyDepth(rs::GfxContext* ctx);

		virtual Node*			clone(Node*n);

		virtual void			im() override;
		void					imBlend();

		virtual void			drawFilteringResult(rs::GfxContext* ctx) override;
		virtual void			serialize(Pasta::JReflect& jr, const char* _name) override;

		virtual double			getValue(rs::TVar valType);
		virtual double			setValue(rs::TVar valType, double val);

		void					blendAdd();

		void					setShaderTimeOffset(float f);

	protected:
		rd::Vars				shaderValues;
		
		Pasta::u32				computeShaderFlags(rs::GfxContext* ctx);
		Pasta::ShaderProgram *	pickShader(rs::GfxContext* ctx);

		virtual void			bindShader(rs::GfxContext* ctx);
		virtual void			bindTexture(rs::GfxContext* ctx, Pasta::Texture* tex, int slot = 0);

		/**
		* returns false if we should abort drawing
		*/
		void					applyBlendmode(rs::GfxContext* ctx);
		virtual bool			drawPrepare(rs::GfxContext* ctx);
		virtual void			drawSubmitGeometry(rs::GfxContext* ctx);
		virtual void			drawCleanup(rs::GfxContext* ctx);

		virtual bool			shouldRenderThisPass(rs::GfxContext* ctx);

		double					getFinalAlpha();
	protected:
		
	};
}

