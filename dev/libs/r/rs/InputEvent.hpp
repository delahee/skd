#pragma once

#include <algorithm>
#include <string>

#include "1-input/InputEnums.h"
#include <optional>

namespace rs{

	enum MouseButton {
		BUTTON_NONE = 0,
		BUTTON_LEFT = 1,
		BUTTON_RIGHT = 2,
		BUTTON_MIDDLE = 4
	};

	enum class InputEventKind : u32 {
		EIK_Push		= 0, //mouse clicked
		EIK_Release,		 //mouse released
		EIK_Move,			// mouse clicked and moved
		EIK_Enter,			//mouse entered once
		EIK_Over,			// mouse overing
		EIK_Out,			// mouse exit zone
		EIK_Wheel,			// wheel used

		EIK_Focus,	
		EIK_FocusLost,

		EIK_KeyDown,		//pad
		EIK_KeyUp,		//pad

		EIK_Simulated,	

		EIK_Character	,//keyboard
	};

	class InputEvent {

	public:
		InputEventKind kind		= InputEventKind::EIK_Simulated;

		std::optional<bool>		stopPropagation	= std::nullopt;

		float relX				= 0.f;
		float relY				= 0.f;
		float wheelDelta		= 0.f;
		

		/// <summary>
		/// mask on BUTTON_NONE / BUTTON_LEFT / BUTTON_MIDDLE / BUTTON_RIGHT
		/// </summary>
		int button				= BUTTON_NONE;
		int touchId				= 0;
		int keyCode				= 0;
		int charCode			= 0;
		int duration			= 0;

		Pasta::Key native		= Pasta::Key::PAD_BEGIN_ENUM;

		InputEvent(InputEventKind k, float x = 0., float y = 0.);

		std::string toString();
		~InputEvent();
		InputEvent(const InputEvent & elem);
		bool isGeometricEvent();

		bool isLeftClick() { return button & BUTTON_LEFT; };
		bool isRightClick() { return button & BUTTON_RIGHT; };
		bool isMiddleClick() { return button & BUTTON_MIDDLE; };
	};
}