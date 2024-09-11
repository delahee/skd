#pragma once

#include "rs/ITweenable.hpp"
#include "rd/Vector3d.hpp"
#include "rd/Sig.hpp"
#include "rd/Vars.hpp"
#include "r2/Bounds.hpp"

enum NodeFlags {
	NF_DRAW_CHILDREN_FIRST = 1 << 0,
	NF_MANUAL_MATRIX = 1 << 1,
	NF_UTILITY = 1 << 2,
	NF_ORIGINATES_FROM_POOL = 1 << 3,
	NF_SKIP_CHILD_SERIALIZATION = 1 << 4,
	NF_CUSTOM_POOLING = 1 << 5,//behave like pools except it is not a rd::pools, preempts NF_ORIGINATES_FROM_POOL to avoid custom pooling pbs
	NF_IN_POOL = 1 << 6,//to avoid double destruction safely
	NF_SKIP_DRAW = 1 << 7,//skip draw for content reason ( and not visible/cull but because content is meant to be empty
	NF_SKIP_HIERACHICAL_ALPHA = 1 << 8,//skip alpha cumulation
	NF_SKIP_SERIALIZATION = 1 << 9,
	NF_SKIP_DESTRUCTION = 1 << 10,//destroy won't do anything, delete it yourself
	NF_SKIP_ELEMENT_SERIALIZATION = 1 << 11,
	NF_CULLED = 1 << 12,
	NF_SKIP_MEASURES_FOR_BOUNDS = 1<<13,
	NF_ALPHA_UNDER_ZERO_SKIPS_DRAW = 1<<14,

	NF_EDITOR_PROTECT = 1 << 16,
	NF_EDITOR_FROZEN = 1 << 17,
	NF_EDITOR_HIDE_METADATA = 1 << 18,
	NF_EDITOR_HIDE_RENDERING = 1 << 19,

	NF_ENGINE_HELPER = 1 << 20,
	NF_ENGINE_DOCKED = 1 << 21,

	NF_USER_FLAGS = 1 << 24,
};

namespace Pasta {
	struct JReflect;
}

namespace rd {
	class Agent;
}

enum class NodeType : u32 {//for serialization purposes
	NT_NODE,
	NT_TEXT,
	NT_BATCH,
	NT_GRAPHICS,
	NT_INTERACT,
	NT_SCENE,
	NT_SCISSOR,
	NT_BMP,
	NT_ABMP,
	NT_AUDIO2D,
	NT_VIDEO,
	NT_PATCH,
	NT_RICH_TEXT,

	NT_SLICE3,
	NT_SLICE9,
	NT_VERTEX_ARRAY,

	NT_RUI_BUTTON = 1<<6,//rui after this
	NT_RUI_FAKE_BUTTON,
	NT_RUI_MENU,

	NT_3D_START = 1<<7,

	NT_NODE3D = NT_3D_START,
	NT_GRAPHICS3D,
	

	NT_USER_TYPE = 1<<8,
};

namespace r2 {
	class NodeAgent;
	class BatchElem;
	class Filter;
	class Scene;
	class NodeAgentList;

	struct TRS {
		rd::Vector3d xyz;
		rd::Vector3d scaleXscaleYRotation;
	};

	typedef rd::MsgChan<const Vector2&> ResizeMessageChannel;
	class Node : public rs::ITweenable {

		////////////////
		////////////////
	public:
		bool					visible = true;
		bool					trsDirty = true;

		r::u32					nodeFlags = 0;

		double					x = 0.0;
		double					y = 0.0;
		double					z = 0.0;
		double					scaleX = 1.0;
		double					scaleY = 1.0;
		double					rotation = 0.0;
		double					alpha = 1.0;

		Node*					parent = nullptr;
		r2::Filter*				filter = nullptr;

		uint64_t				uid = 0;

		Str						name;

		rd::Vars				vars;
		eastl::vector<Node*>	children;

		rd::Sig					onDestruction;
		rd::Sig					onDeletion;
		rd::Sig					onDispose;

		rd::MsgChanVec2*		sigOnResize = nullptr;

		double*					pos() { return &x; trsDirty = true; };

		void					setPosVec2(const r::Vector2& pos) { this->x = pos.x; this->y = pos.y; this->trsDirty = true; };
		void					setPosVec3(const r::Vector3& pos) { this->x = pos.x; this->y = pos.y; this->z = pos.z; this->trsDirty = true; };
		void					setPos(double x, double y, double z = 0.0f) { this->x = x; this->y = y; this->z = z; this->trsDirty = true; };

		void					setUniformScale(double x);
		void					setScale(double x, double y)	{ this->scaleX = x; this->scaleY = y; this->trsDirty = true; };
		void					setScaleX(double x)				{ this->scaleX = x; this->trsDirty = true; };
		void					setScaleY(double y)				{ this->scaleY = y; this->trsDirty = true; };

								Node(Node* parent = nullptr);
		virtual					~Node();

		void					syncMatrix();
		void					syncAllMatrix(bool force = false);

		/**
		 * Be aware that when update is called children will not be processed ( for ex for TRS considerations ) after
		 * So ensure you emit debug draw commands before calling this or super:: stuff
		 */
		virtual void			update(double dt);

		/**
		dispose()
		- can be called multiple times
		- called before effective destruction
		- may leave the instance in a nearly reusable state albeit totally empty
		- destroy all it's inner content or set it to initial and free resources as much as possible
		- after dispose we should be able to restart the instance for reuse
		- cuts the instance from the hierarchy
		- if you have content to manually dispose or clean, do it before disposal to avoid double dispose
		*/
		virtual void			dispose();

		/**
		reset()
		- calls destroyAllChildren
		- can be called multiple times
		- called before effective reallocation
		- may leave the instance in a shallow as new state
		- after reset we should be able to use the instance out of the box for anything pertaining
		- does not cut from parent so far
		*/
		virtual	void			reset();

		//how this nodes solves his own draw
		virtual void			draw(rs::GfxContext* ctx);

		//how this nodes solved it content draw ( nests ::draw() )
		virtual void			drawContent(rs::GfxContext* ctx);

		//wraps drawContent for filtering 
		virtual void			drawFiltered(rs::GfxContext* ctx);

		virtual void			drawFilteringResult(rs::GfxContext* ctx);

		//when recursive draw hits calls this
		virtual void			drawRec(rs::GfxContext* ctx);

		virtual Pasta::Matrix44	getLocalMatrix();
		Pasta::Matrix44			getGlobalMatrix();

		Pasta::Matrix44			getRelativeMatrix(r2::Node* to);

		virtual	void			onAddChild(Node* c) {};
		void					addChild(Node* c);
		void					addChildAt(Node* c, int pos);

		void					putBehind(Node* el);
		void					putInFrontOf(Node* el);

		void					toBack();
		void					backward(int nb);
		void					toFront();

		void					detach();

		virtual void			onEnterHierarchy();
		virtual void			onExitHierarchy();

		bool					hasChild(Node* c);
		bool					hasChildRec(Node* c);

		virtual void			onRemoveChild(Node* c) {};
		bool					removeChild(Node* c);

		void					setChildIndex(Node* c, int newPos);

		int						getChildIndex(Node* c) const;
		int						nbChildren() const { return children.size(); };
		int						getDepthInHierarchy();

		virtual void			setWidth(float width);
		virtual void			setHeight(float height);

		void					setSize(float width, float height);
		void					setSizeVec2(const Vector2& v);

		virtual double			width();
		virtual double			height();

		virtual Scene*			getScene();

		//single bhv entry point, if you want a full vector of behvaiours, inject an agent please
		std::function<void(Node*)> bhv;

		virtual double			getValue(rs::TVar valType) override;
		virtual double			setValue(rs::TVar valType, double val) override;

		void					drawTo(Pasta::Texture* t, Pasta::FrameBuffer* _fb = nullptr, Scene* _s = nullptr);
		//doesn't really mean anything semantic wise
		//Bounds				getSize();
		r::Vector2				getSize();
		r::Vector2				getPos();

		r::Vector3				getPosVec3() { return Vector3(x, y, z); };

		r::Vector2				localToGlobal(Pasta::Vector2 pt);
		r::Vector2				globalToLocal(Pasta::Vector2 pt);

		/**
		Only process Node as a leaf
		Get the bounds relative to the node coordinate system
		If you want to force bounds size, do it here
		*/
		virtual Bounds			getMyLocalBounds();

		virtual Bounds			getMeasures(bool forFilters);

		/**
		Collect my bounds and
		Collect all children bounds
		if filtered returns the filtered size
		syncAllMatrix is not called because it is costly so ensure your matrixes are up to date
		*/
		Bounds					getLocalBounds();

		/**
		* Retrieves bounds with hierarchy INCLUDED
		* syncAllMatrix is not called because it is costly so ensure your matrixes are up to date
		*/
		virtual Bounds			getBounds(r2::Node* relativeTo = nullptr);

		Bounds					getScreenBounds();

		/**
		* Retrieves bounds with hierarchy EXCLUDED
		*/
		virtual Bounds			getMyBounds(r2::Node* relativeTo = nullptr);

		//the & is important to avoid too many function assign calls
		void					traverseLastChildFirst(std::function< void(r2::Node*)>& visit);
		void					traverse(std::function<void(r2::Node*)> visit);

		void					setName(const char* n);
		void					setAlpha(float alpha);
		r2::Node*				findByName(const char * name);
		r2::Node*				findByTag(const char * tag);

		r2::Node*				findByUID(r::u64 uid);
		virtual void			findByUID(r::u64 uid, r2::Node*& node, r2::BatchElem*& elem);

		r2::Node*				find( std::function<bool(r2::Node*n)>);

		virtual void			disposeAllChildren();
		virtual void			deleteAllChildren();
		virtual void			removeAllChildren();
		virtual void			destroyAllChildren();
		static Node* dummy;

		void					setX(double _x) { x = _x; trsDirty = true; };
		double					getX() { return x; };


		TRS						getTRS();
		void					resetTRS();
		void					invalidateTRS();

		void					setTRS(double x, double y, double scaleX, double scaleY, double rotation, double z);
		void					setTRS(const TRS& trs);

		virtual  r2::Node*		clone(r2::Node* n = nullptr);

		void					copyMatrix(r::Matrix44& _mat);
		void					overrideMatrix(const r::Matrix44& _mat);
		void					restoreMatrix(const r::Matrix44& _mat);
		void					setToLocalMatrix();

		void					syncChildrenMatrix();

		virtual void			im();

		virtual void			serialize(Pasta::JReflect& f, const char* name = nullptr);
		void					serializeChildren(Pasta::JReflect& jr);
		void					serializeComponents(Pasta::JReflect& jr);
		static void				serializeArray(Pasta::JReflect& jr, eastl::vector<r2::Node*>& arr, u32 size, const char* name);

		inline bool				isCulled() const { return nodeFlags & NF_CULLED; };
		inline bool				isVisible() const { return visible; };
		inline bool				isUtility() const { return nodeFlags & NF_UTILITY; };
		inline bool				isPooledIn() const { return nodeFlags & NF_IN_POOL; };
		inline bool				isCustomPooled() const { return nodeFlags & NF_CUSTOM_POOLING; };

		virtual	NodeType		getType() const { return NodeType::NT_NODE; };

		//destroy resources and sends back instance to memory liberation
		// basically
		// call dispose
		// - if need be send to pool
		// - if need be delete(obj) from mem
		virtual void			destroy();

		virtual void			toPool();
		virtual void			onResize(const r::Vector2 &_newScreenSize);

		//set size then sync scale
		void					constraintHeight(float h);
		//set size then sync scale
		void					constraintWidth(float w);

		static
		eastl::vector<r2::Node*>collect( r2::Node * tgt);

		virtual void			show() { visible = true; };
		virtual void			hide() { visible = false; };

		void					setTag(const char * tag);
		bool					hasTag(const char * tag) const;

		static r2::Node*		fromPool(r2::Node*parent);

		void					addComponent(r2::NodeAgent* a);

		inline void addOnResize(rd::MsgChanVec2::Func f);
	private:
		static
			void _collect(r2::Node* tgt, eastl::vector<r2::Node*>* cur );
	////////////////
	////////////////
	protected:

		Pasta::Matrix44 mat;

		double	_x			= 0.0;
		double	_y			= 0.0;
		double	_z			= 0.0;
		double	_scaleX		= 0.0;
		double	_scaleY		= 0.0;

		double	_rotation		= 0.0;
		
		r2::NodeAgentList*agl	= 0;

		//float	dummy0		= 0.0f;
		//float	dummy1		= 0.0f;

		void _traverse(std::function<void(r2::Node*)>& visit);

	public:
		inline const Pasta::Matrix44&	getLastMatrix() const {
			return mat;
		};

	};

	
	inline void r2::Node::addOnResize(rd::MsgChanVec2::Func f) {
		if (!sigOnResize)
			sigOnResize = new rd::MsgChanVec2();
		sigOnResize->add(f);
	};
}

