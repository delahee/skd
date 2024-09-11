#include "stdafx.h"

using namespace std;
rs::InputEvent::InputEvent(InputEventKind k, float x, float y) {
	kind = k;
	relX = x;
	relY = y;
}

static std::string kindToString(InputEventKind iek) {
	switch (iek){
	case InputEventKind::EIK_Push:		return "EIK_Push";
	case InputEventKind::EIK_Release:	return "EIK_Release";
	case InputEventKind::EIK_Move:		return "EIK_Move";
	case InputEventKind::EIK_Enter:		return "EIK_Enter";
	case InputEventKind::EIK_Over:		return "EIK_Over";
	case InputEventKind::EIK_Out:		return "EIK_Out";
	case InputEventKind::EIK_Wheel:		return "EIK_Wheel";
	case InputEventKind::EIK_Focus:		return "EIK_Focus";
	case InputEventKind::EIK_FocusLost:	return "EIK_FocusLost";
	case InputEventKind::EIK_KeyDown:	return "EIK_KeyDown";
	case InputEventKind::EIK_KeyUp:		return "EIK_KeyUp";
	case InputEventKind::EIK_Simulated:	return "EIK_Simulated";
	case InputEventKind::EIK_Character:	return "EIK_Character";
	default:
		break;
	}
	return"";
}

std::string rs::InputEvent::toString() {
	return kindToString(kind) + "[" + std::to_string(std::rint(relX)) + "," + std::to_string(std::rint(relY)) + "]";
}

rs::InputEvent::~InputEvent() {

}

rs::InputEvent::InputEvent(const InputEvent & elem) : InputEvent(elem.kind, elem.relX, elem.relY) {
	stopPropagation = elem.stopPropagation;
	wheelDelta = elem.wheelDelta;
	button = elem.button;
	touchId = elem.touchId;
	keyCode = elem.keyCode;
	charCode = elem.charCode;
	duration = elem.duration;
	native = elem.native;
	origin = elem.origin;
}

bool rs::InputEvent::isGeometricEvent() {
	switch (kind) {
	default: return false;
	case InputEventKind::EIK_Push:
	case InputEventKind::EIK_Release:
	case InputEventKind::EIK_Move:
	case InputEventKind::EIK_Over:
	case InputEventKind::EIK_Out:
	case InputEventKind::EIK_Wheel:
	case InputEventKind::EIK_Focus:
	case InputEventKind::EIK_FocusLost: return true;
	};
}

InputOrigin rs::InputEvent::resolveOrigin(Pasta::ControllerType ct){
	switch (ct) {
		case Pasta::ControllerType::CT_KEYBOARD:
			return InputOrigin::Keyboard;
		case Pasta::ControllerType::CT_MOUSE:
			return InputOrigin::Mouse;
		default:
			return InputOrigin::Pad;
	}
}
