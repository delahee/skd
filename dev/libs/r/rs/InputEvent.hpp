#pragma once

#include "1-input/InputEnums.h"

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

	enum InputOrigin : int{
		Unknown,
		Mouse,
		Touch,
		Pad,
		Keyboard,
		Simulation,
	};

	class InputEvent {

	public:
		rs::InputEventKind		kind		= rs::InputEventKind::EIK_Simulated;
		rs::InputOrigin			origin		= rs::InputOrigin::Simulation;

		std::optional<bool>		stopPropagation	= std::nullopt;

		float relX				= 0.f;
		float relY				= 0.f;
		float wheelDelta		= 0.f;

		int button				= BUTTON_NONE;
		int touchId				= 0;
		int keyCode				= 0;
		int charCode			= 0;
		int duration			= 0;

		Pasta::Key native		= Pasta::Key::PAD_BEGIN_ENUM;

		InputEvent() {};
		InputEvent(InputEventKind k, float x = 0., float y = 0.);
		InputEvent(const InputEvent& elem);

		std::string	toString();

				~InputEvent();
		bool	isGeometricEvent();

		bool	isLeftClick() { return button & BUTTON_LEFT; };
		bool	isRightClick() { return button & BUTTON_RIGHT; };
		bool	isMiddleClick() { return button & BUTTON_MIDDLE; };

		static rs::InputOrigin resolveOrigin(Pasta::ControllerType ct);
	};
}