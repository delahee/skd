#pragma once

namespace r2 { 
	class Batch; 
	class Bitmap; 
}

enum class BeType : u32 {
	BET_BATCH_ELEM,
	BET_ABATCH_ELEM,
	BET_SUB_BATCH_ELEM,
	BET_SUB_ABATCH_ELEM,

	BET_USER = 1 << 16,
};

namespace r2 {
	class Batch;
	class BatchElem : public rs::ITweenable {
	public:
		friend class			Batch;
		
		//please consider aligning vars on 32bits, beware header order is designed, don't reorder without cache awareness please
		bool					useSimpleZ = true;
		bool					visible = true;
		bool					ownsTile = false;
		bool					destroyed = false;
		//if you had a bool, please pad on 32bits 

		r::u32					beFlags = 0;//see NF_flags
		r::TransparencyType		blendmode = r::TransparencyType::TT_INHERIT;

		//we can use float here because it's up to matrices to build group translation so we shouldn't suffer from approx here
		float					x = 0.0f;
		float					y = 0.0f;
		float					z = 0.0f;
		float					alpha = 1.0f;

		float					scaleX = 1.0f;
		float					scaleY = 1.0f;
		float					rotation = 0.0f;
		float					zTopOffset = 0.0f;
		float					zBottomOffset = 0.0f;
		
		double					priority = 0.0;
		uint64_t				uid = 0;

		Batch*					batch = nullptr;
		Tile*					tile = nullptr;
		BatchElem*				next = nullptr;
		BatchElem*				prev = nullptr;
		
		rd::Vars				vars;//basically a ptr
		
		Str						name;
		r::Color				color;

								BatchElem();
								BatchElem(Tile* tile, Batch* parent = nullptr, double priority = 0);

								//the copy is disconnected from batch
								BatchElem(const BatchElem& src);
		virtual					~BatchElem();

		//clear and ready to go
		virtual void			reset();
		virtual void			remove();
		virtual void			dispose();
		virtual void			destroy();
		virtual void			pushQuad(eastl::vector<float>& vertices, eastl::vector<u32>& indices);
		virtual void			pushQuadNormals(eastl::vector<float>& vertices, eastl::vector<u32>& indices);

		void					setZOffset(float top, float bottom) { zTopOffset = top; zBottomOffset = bottom; };
		void					detach() { remove(); };
		double					width();
		double					height();

		void					setWidth(double w);
		void					setHeight(double Float);
		void					setSize(double w, double h);
		void					setSize(double sz) { setSize(sz, sz); };

		float* pos() { return &x; }
		void					setPos(double x, double y) { this->x = x; this->y = y; }
		void					setScale(double x, double y) { this->scaleX = x; this->scaleY = y; }
		void					scale(double v) { this->scaleX *= v; this->scaleY *= v; };
		virtual void			setCenterRatio(double px = 0.5, double py = 0.5);

		void					setPriority(double p);
		inline constexpr double	getPriority() const { return priority; };
		void					incrPriority(double val) { setPriority(priority = val); };
		void					clear();

		virtual double			getValue(rs::TVar valType);
		virtual double			setValue(rs::TVar valType, double val);

		virtual void			update(double dt) {}; //only here for customization purpose

		void					setTile(r2::Tile* t, bool own = false);
		Tile*					getTile() { return tile; };

		void					setName(const char* _name);

		inline Pasta::Texture* getTexture() {
			if (!tile) return nullptr;
			return tile->getTexture();
		};

		void					blendAdd();
		void					blendAlpha();


		virtual r2::BatchElem*	clone(r2::BatchElem* nu = nullptr) const;
		virtual void			im();

		virtual void			getLocalBounds(Bounds& out) const;

		void					getBounds(Bounds& b, r2::Node* relativeTo = nullptr) const;
		Bounds					getBounds(r2::Node* relativeTo = nullptr) const;

		inline bool				isCulled() { return beFlags & NF_CULLED; };
		inline bool				isVisible() { return visible; };
		inline bool				isUtility() { return beFlags & NF_UTILITY; };

		bool					shouldRender(rs::GfxContext* _g);
		virtual BeType			getType() const;
		virtual void			serialize(Pasta::JReflect& jr, const char* _name = nullptr);

		void					load(const r2::Bitmap* src);
		static BatchElem*		fromPool(rd::TileLib* lib, const char * group, r2::Batch* batch = nullptr);
		static BatchElem*		fromLib(rd::TileLib* lib, const char* group, r2::Batch* batch = nullptr) {
			return r2::BatchElem::fromPool(lib, group, batch);
		};
	};
}

extern bool BatchElemTest;