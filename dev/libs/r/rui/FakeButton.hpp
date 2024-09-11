#pragma once

#include "rd/Agent.hpp"
#include "r2/Node.hpp"
#include "rui/IInteractive.hpp"
#include "rui/ClickHoldDetector.hpp"

namespace rui {
	class Menu;
	
	//when you want to mock content without button constraints and specifics
	class FakeButton : public IInteractive, public r2::Node {
		typedef r2::Node		Super;
		typedef IInteractive	SuperInterface;
	public:
							FakeButton(r2::Node * parent = nullptr);
		virtual				 ~FakeButton();

		rd::AgentList		al;

		bool				hovered = false;
		bool				focused = false;
		bool				greyed = false;
		bool				enabled = true;

		rui::Menu*			menu = 0;

		rs::InputEvent		ev;
		rd::Sig				sigOnEnter;
		rd::Sig				sigOnOut;
		rd::Sig				sigOnOver;
		rd::Sig				sigOnFocus;
		rd::Sig				sigOnBlur;
		rd::Sig				sigOnDispose;
		rd::Sig				sigOnClick;
		rd::Sig				sigOnMouseButtonDown;
		rd::Sig				sigOnMouseButtonUp;

		rd::Sig				onPack;

		void				setDebug(bool onOff);

		void				pack();

		virtual	void		im()override ;

		r2::Interact*		inter = nullptr;

	public:

		//event IInteractive api
		virtual bool		isHovered();
		virtual void		setHovered(bool _onOff); 
		
		virtual bool		isEnabled();
		virtual void		setEnabled(bool _onOff);

		virtual bool		isFocused();
		virtual void		setFocused(bool _onOff);

		virtual bool		isGreyed() const;
		virtual void		setGreyed(bool _onOff);

		virtual bool		isVisible();
		virtual void		setVisible(bool _onOff);

		virtual bool		hasParent();
		virtual bool		canInteract();

		virtual void		onMouseEnter();
		virtual void		onMouseOut();
		virtual void		onMouseOver();
		virtual void		onFocus();
		virtual void		onFocusLost();

		virtual void		doClick();
		virtual r::Vector2	getPosition();
		virtual r::Vector2	getAbsolutePosition();

		virtual r::Vector2	getSize();
		virtual void		serialize(Pasta::JReflect& f, const char* name = nullptr);
		virtual	NodeType	getType()const override { return NodeType::NT_RUI_FAKE_BUTTON; };

	public:
		virtual void		update(double dt);
		virtual void		dispose();


		void				setupInter();
		void				setupSig(); 

	};
}
	