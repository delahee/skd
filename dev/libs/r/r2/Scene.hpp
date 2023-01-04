#pragma once

#include "Node.hpp"
#include "1-graphics/Graphic.h"
#include "1-graphics/Texture.h"

#include "rs/InputEvent.hpp"
#include "r2/Interact.hpp"
#include "rd/Agent.hpp"
#include <vector>

using namespace rs;

typedef std::function<void(Pasta::Graphic * g)> RenderOp;
typedef std::function<void(rs::GfxContext * g)> RenderOpCtx;
typedef std::function<void(double dt)> UpdateOp;

namespace r2 {
	
	class Scene;

	struct InputDep {
		r2::Scene*	sc;
		bool		before = true;
	};

	//warning don't try to "filter" a scene, this is evil, please use a capture to perform this form of dark art
	class Scene : public Node, public IEventListener {
	public:
		
		bool							fixedSize = false;
		bool							doClear = true;
		r::Color						clearColor;

		r::Vector3						cameraPos;
		r::Vector3						cameraScale = r::Vector3(1,1,-1);

		r::Matrix44 					rotationMatrix = r::Matrix44::identity;

	public:
										Scene( r2::Node * parent = nullptr );
		virtual							~Scene();

		virtual void					dispose() override;

		void							removeEventManagement();
		float&							getPanX();
		float&							getPanY();
		void							setPan(float x, float y);
		Vector2							getPan() { return cameraPos; };

		float&							getZoomX();
		float&							getZoomY();

		void							setZoom(float xy) ;
		void							setZoom(float x, float y);

		//camera depth now goes from - 1/factor to 1/factor
		void							setDepthScale(float factor);

		static const int VCamPosX		= VCustom0;
		static const int VCamPosY		= VCustom1;
		static const int VCamPosZ		= VCustom2;

		static const int VCamScaleX		= VCustom3;
		static const int VCamScaleY		= VCustom4;
		static const int VCamScaleZ		= VCustom5;

		virtual double					getValue(rs::TVar valType) override;
		virtual double					setValue(rs::TVar valType, double val) override;

		inline void						setCameraScale(double sx, double sy) { cameraScale.x = sx; cameraScale.y = sy; };

		float							getDepthRange();
		//camera depth now goes from - range to range
		void							setDepthRange(float range);

		void							overrideDimensions(float w, float h);

		virtual void					update(double dt);
		
		virtual void					stdMatrix( rs::GfxContext * ctx, int w=-1, int h=-1 );
		virtual void					render( Pasta::Graphic *g );
		void							checkEvents();
		void							syncViewMatrix();

		virtual double					width();
		virtual double					height();

		float							windowWidth();
		float							windowHeight();

		virtual Scene*					getScene() override;

		/**
		* Beware this function is general purpose, it does not set scissoring
		*/
		void							drawInto(rs::GfxContext * _g, r2::Node * node, Pasta::Texture * t, Pasta::FrameBuffer * fb = nullptr, rs::Pass pass = Pass::Basic);

		virtual rs::InputEvent			transformEvent(rs::InputEvent& ev);
		virtual bool					handleEvent(rs::InputEvent & ev);

		bool							computeNodeVisibility(r2::Node * node);

		void							addEventTarget( r2::Interact * inter );
		void							removeEventTarget(r2::Interact * inter);

		r2::Interact *					getCurrentFocus()	{ return currentFocus; };
		r2::Interact *					getCurrentOver()	{ return currentOver; };

		//should apply those in user space code, not applying this can be intentionnal
		//void onResize(ns);

		/*
		* cancels olds focus via event
		* setup the new
		* triggers the focus event
		*/
		void								setCurrentFocus(r2::Interact * inter, bool triggerCbk = true);

		/*
		* cancels olds over via event
		* setup the new
		* triggers the out event
		*/
		void								setCurrentOver(r2::Interact * inter, bool triggerCbk=true);

		virtual void						im() override;
		
		std::vector< r2::Interact * >		interacts;
		std::vector< r2::Interact * >		_tmpInteracts;

		bool								areDimensionsOverriden = false;
		float								sceneWidth = 0.0;
		float								sceneHeight = 0.0;
		Node*								getByName(const std::string & name);

		r2::InputDep&						registerInputDependency(r2::Scene * sc);
		void								unregisterInputDependency(r2::Scene * sc);

		std::vector<UpdateOp>				preUpdateOps;
		std::vector<UpdateOp>				postUpdateOps;

		std::vector<RenderOp>				preRenderOps;
		std::vector<RenderOpCtx>			preClearRenderOps;
		std::vector<RenderOpCtx>			postClearRenderOps;
		std::vector<RenderOpCtx>			postRenderOps;

		AgentList							al;

		virtual void						serialize(Pasta::JReflect & jr, const char * _name = nullptr) override;

		virtual	NodeType					getType() const override { return NodeType::NT_SCENE; };

		const Matrix44&						getViewMatrix() const { return viewMatrix; };

		bool								DEBUG_EVENT = false;

	protected:
		r2::Interact *						currentFocus = nullptr;
		r2::Interact *						currentOver = nullptr;

		virtual void						onEvent( InputEvent & ev ) override;
		std::vector<InputEvent>				pendingEvents;
		std::vector<InputDep>				deps;


	private:
		//if you really need this, consider using setCameraScale or manipulating scale by hand, no support is giver right now
		using r2::Node::setScale;

	//protected: but public for hacking purpose
	public:
		r::Matrix44							projMatrix;
		r::Matrix44							viewMatrix;
	};


	class EarlyDepthScene : public Scene {
	public:
		bool skipBasic = false;
		bool skipClip = false;
		virtual void render(Pasta::Graphic *g) override;
		virtual void im() override;
		virtual void serialize(Pasta::JReflect & jr, const char * _name = nullptr) override;
	};
}


