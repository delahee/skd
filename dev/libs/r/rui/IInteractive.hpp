#pragma once
#include "../rs/InputEvent.hpp"
#include "1-graphics/geo_vectors.h"

namespace rui {
	class IInteractive {//see rui::button for reference implemenntation
	public:
		virtual bool isHovered()					= 0;
		virtual void setHovered(bool _onOff)		= 0;

		virtual bool isEnabled()					= 0;
		virtual void setEnabled(bool _onOff)		= 0;

		virtual bool isFocused()					= 0;
		virtual void setFocused(bool _onOff)		= 0;

		virtual bool isGreyed()						= 0;
		virtual void setGreyed(bool _onOff)			= 0;

		virtual bool isVisible()					= 0;
		virtual void setVisible(bool _onOff)		= 0;

		virtual bool hasParent()					= 0;
		virtual bool canInteract()					= 0;

		//event simulators
		virtual void onMouseOut(rs::InputEvent &)	= 0;
		virtual void onMouseOver()					= 0;
		virtual void onFocus()						= 0;
		virtual void onFocusLost()					= 0;

		virtual void		doClick()				= 0;
		virtual r::Vector2	getPosition()			= 0;
		virtual r::Vector2	getSize()				= 0;
	};
}