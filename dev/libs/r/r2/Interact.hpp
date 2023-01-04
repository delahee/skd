#pragma once

#include "Node.hpp"
#include "../rs/InputEvent.hpp"

namespace r2 {

	typedef std::vector< std::function<void(rs::InputEvent &)> > EventHandlers;

	/**
	* general ideas
	* collect all interacts in depth order
	*/
	class Interact : public r2::Node {

	public:
		float				rectWidth = 0.f;
		float				rectHeight = 0.f;

	public:
		bool				enabled					= true;
		float				isMouseDown				= false;
		float				mouseClickedDuration	= -1.0f;

		bool				wasInside				= false;
		bool				disableBounds			= false;

		r2::EventHandlers	onMouseButtonDowns;
		r2::EventHandlers	onMouseButtonUps;

		r2::EventHandlers	onMouseMoves	;//mouse moves and is clicked
		r2::EventHandlers	onMouseEnters	;
		r2::EventHandlers	onMouseOvers	;//mouse moves over something
		r2::EventHandlers	onMouseOuts		;
		r2::EventHandlers	onMouseWheels	;

		r2::EventHandlers	onFocuses		;
		r2::EventHandlers	onFocusLosts	;

		r2::EventHandlers	onKeyUps		;
		r2::EventHandlers	onKeyDowns		;

		r2::EventHandlers	onChars			;

	public:
							Interact(r2::Node * parent = nullptr);
							Interact(float width, float height, r2::Node * parent = nullptr);
		virtual				~Interact();

		virtual	void		dispose();

		void				onEnterHierarchy() override;
		void				onExitHierarchy() override;

		bool				hasFocus();

		void				blur( bool callEvents = true );//but don't call event
		void				focus( bool callEvents = true );

		void				handleEvent(rs::InputEvent & ev);
	
		virtual Bounds		getMyLocalBounds() override;
		virtual Bounds		getMeasures(bool forFilters) override;

		virtual	void		computeFocusLost(rs::InputEvent & ev);
		virtual void		computeFocus(rs::InputEvent & ev);
		
		virtual void		computePush(rs::InputEvent & ev);
		virtual void		computeRelease(rs::InputEvent & ev);

		virtual void		computeMove(rs::InputEvent & ev);
		virtual void		computeOut(rs::InputEvent & ev);
		virtual void		computeOver(rs::InputEvent & ev);
		virtual void		computeWheel(rs::InputEvent & ev);

		virtual void		computeKeyUp(rs::InputEvent & ev);
		virtual void		computeKeyDown(rs::InputEvent & ev);
		virtual void		computeChars(rs::InputEvent & ev);
		virtual void		computeEnters(rs::InputEvent & ev);

		virtual void		update(double dt) override;
	
		virtual	NodeType	getType()const override { return NodeType::NT_INTERACT; };
		virtual void		serialize(Pasta::JReflect& jr, const char* _name) override;
		virtual void		im() override;

		r2::Scene*			assignedScene = 0;
		void				ensureScene();

		void				fitToParent();
		Vector2				getRelativeToParent(const Vector2& v);

		std::function<bool(const rs::InputEvent& ev)>
							doAcceptEventFunc;
		bool				doesAcceptEvent(const rs::InputEvent& ev);
	};//End Interact
}//End r2