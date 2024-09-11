#pragma once

#include "rd/Agent.hpp"
#include "r2/Node.hpp"
#include "rui/IInteractive.hpp"
#include "rui/ClickHoldDetector.hpp"

namespace rui {
	class Menu;
	//some sort of referential implementation of the thing...
	class Button : public IInteractive, public r2::Node {
		typedef r2::Node		Super;
		typedef IInteractive	SuperInterface;
	public:
		friend				rui::ClickHoldDetector;
							/**
							 * Button detects if parent is a menu,
							 * if it is one, then button is automatically added to navigation
							 * if it is not one,
							 * to add to navigation sur Menu::addButton();
							 */
							Button(const char* label, rd::Font * fnt,r2::Node * parent = nullptr);
							Button(const std::string& label, rd::Font* fnt, r2::Node* parent = nullptr) : Button(label.c_str(), fnt, parent) {};
							Button(const Str& label, rd::Font* fnt, r2::Node* parent = nullptr) : Button(label.c_str(), fnt, parent) {};

		virtual				 ~Button();

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
		rd::Sig				sigOnGreyed;

		rd::Sig				onPack;

		void				setActiveColor(int rgb, float alpha);
		void				setActiveColor(r::Color col);

		void				setGreyedColor(int rgb, float alpha);
		void				setGreyedColor( r::Color col );

		void				setBackgroundColor(const r::Color &col);
		void				setFocusedColor(r::Color col);

		void				updateColor();

		void				setLabel(const char * txt );

		r2::Text		*	getText() { return label; };
		r2::Interact	*	getInteract() { return inter; };

		void				setDebug(bool onOff);

		//update interact size to match label;
		void				pack();

		virtual	void		im()override ;
		void				setFont(rd::Font* fnt);
		void				setFontSize(int sz);

		r2::Node*			bg = 0;
		int					interactPadding = 8;
	protected:
		r2::Text		*	label = nullptr;
		r2::Interact	*	inter = nullptr;

		r::Color			activeColor = r::Color(1.f, 1.f, 1.f);
		r::Color			focusedColor = r::Color(1.f, 0.f, 1.f);
		r::Color			greyedColor = r::Color(0.2f, 0.2f, 0.2f);
		r::Color			bgColor = r::Color(0.f, 0.1f, 0.2f,0.95f);
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
		r2::Text*			getLabel() { return label; };
		virtual void		serialize(Pasta::JReflect& f, const char* name = nullptr);
		virtual	NodeType	getType()const override { return NodeType::NT_RUI_BUTTON; };

		virtual void		applyStyle(const rd::Style& st);

	public:
		virtual void		update(double dt);
		virtual void		dispose();

	protected:
		void				setupInter();
		void				setupSig(); 

	};
}
	