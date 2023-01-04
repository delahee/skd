#pragma once

#include "1-input/InputEnums.h"

namespace rs {
	enum InputFlags {
		MENU = 1 << 0,
		PAUSE = 1 << 1,
		ANIM = 1 << 2,
		END = 1 << 3,
		UI_ANIM = 1 << 4,
	};

	class Input {
		public:
			static int flags;

			static bool isKeyboardKeyDown(Pasta::Key kc);
			static bool isKeyboardKeyUp(Pasta::Key kc);
			static bool isKeyboardKeyJustPressed(Pasta::Key kc);
			static bool isKeyboardKeyJustReleased(Pasta::Key kc);

			static bool isKeyDown		(Pasta::ControllerType ct, Pasta::Key kc);
			static bool isKeyUp			(Pasta::ControllerType ct, Pasta::Key kc);
			static bool isKeyJustPressed(Pasta::ControllerType ct, Pasta::Key kc);
			static bool isKeyJustReleased(Pasta::ControllerType ct, Pasta::Key kc);
			static float getKeyValue	(Pasta::ControllerType ct, Pasta::Key kc);

			static bool isDirPressed_Down();
			static bool isDirPressed_Up();
			static bool isDirPressed_Left();
			static bool isDirPressed_Right();

			static bool isDirJustPressed_Down();
			static bool isDirJustPressed_Up();
			static bool isDirJustPressed_Left();
			static bool isDirJustPressed_Right();

			//todo
			//static bool isDirJustPressed_Down();
			//static bool isDirJustPressed_Up();
			//static bool isDirJustPressed_Left();
			//static bool isDirJustPressed_Right();

			static bool isDirReleased_Down();
			static bool isDirReleased_Up();
			static bool isDirReleased_Left();
			static bool isDirReleased_Right();

			static bool onAnyDown();
			static bool onAnyPadDown(Pasta::Key k);
			static bool isAnyPadJustPressed(Pasta::Key k);
			static bool isAnyPadJustReleased(Pasta::Key k);
			static float getWheelValue();
	};

	typedef Pasta::Key					Key;
}