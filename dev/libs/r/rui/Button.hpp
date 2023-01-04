#pragma once

#include "r2/Node.hpp"
#include "rui/IInteractive.hpp"

namespace rui {
	class Menu;
	//some sort of referential implementation of the thing...
	class Button : public IInteractive, public r2::Node {
	public:
							
							Button(const char* label, rd::Font * fnt,r2::Node * parent = nullptr);

							Button(const std::string& label, rd::Font* fnt, r2::Node* parent = nullptr) : Button(label.c_str(), fnt, parent) {
							};

							Button(const Str& label, rd::Font* fnt, r2::Node* parent = nullptr) : Button(label.c_str(), fnt, parent) {
							};

		virtual				 ~Button();

		AgentList*			al = nullptr;

		bool				hovered = false;
		bool				focused = false;
		bool				greyed = false;
		bool				enabled = true;

		Sig					sigOnEnter;
		Sig					sigOnOut;
		Sig					sigOnFocus;
		Sig					sigOnBlur;
		Sig					sigOnDispose;
		Sig					sigOnClick;

		void				setActiveColor(int rgb, float alpha);
		void				setActiveColor(r::Color col);

		void				setGreyedColor(int rgb, float alpha);
		void				setGreyedColor( r::Color col );

		void				setBackgroundColor(r::Color col);

		r2::Text		*	getText() { return label; };
		void				setDebug(bool onOff);
		void				pack();

		r2::Node*			bg = 0;
	protected:
		r2::Text		*	label = nullptr;
		r2::Interact	*	inter = nullptr;
		r::Color			activeColor = r::Color(1.f, 1.f, 1.f);
		r::Color			greyedColor = r::Color(0.2f, 0.2f, 0.2f);
	public:

		//event IInteractive api
		virtual bool		isHovered();
		virtual void		setHovered(bool _onOff); 
		
		virtual bool		isEnabled();
		virtual void		setEnabled(bool _onOff);

		virtual bool		isFocused();
		virtual void		setFocused(bool _onOff);

		virtual bool		isGreyed();
		virtual void		setGreyed(bool _onOff);

		virtual bool		isVisible();
		virtual void		setVisible(bool _onOff);

		virtual bool		hasParent();
		virtual bool		canInteract();
		
		virtual void		onMouseOut(rs::InputEvent &);
		virtual void		onMouseOver();
		virtual void		onFocus();
		virtual void		onFocusLost();

		virtual void		doClick();
		virtual r::Vector2	getPosition();
		virtual r::Vector2	getSize();
		r2::Text*			getLabel() { return label; };
	public:
		virtual void update(double dt);
		virtual void dispose();
	};
}
	