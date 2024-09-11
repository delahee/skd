#include "stdafx.h"

#include "1-input/InputMgr.h"
#include "1-input/InputEnums.h"

#include "Input.hpp"

#include "ri18n/T.hpp"

using namespace Pasta;
using namespace rs;

int rs::Input::flags = 0;

static Pasta::Key keyboardMap(Pasta::Key k) {
	if (ri18n::T::getLang() == "fr") {
		switch (k) {
		case KB_A: return KB_Q;
		case KB_Q: return KB_A;

		case KB_W: return KB_Z;
		case KB_Z: return KB_W;

		default:
			break;
		}
	}
	return k;
}

bool rs::Input::isKeyboardKeyDown(Pasta::Key kc) {
	InputMgr* mgr = Pasta::InputMgr::getSingleton();
	return mgr->keyIsPressed(CT_KEYBOARD, keyboardMap(kc));
}

bool rs::Input::isKeyboardKeyUp(Pasta::Key kc) {
	InputMgr* mgr = Pasta::InputMgr::getSingleton();
	return !mgr->keyIsPressed(CT_KEYBOARD, keyboardMap(kc));
}

bool rs::Input::isKeyboardKeyJustPressed(Pasta::Key kc) {
	InputMgr* mgr = Pasta::InputMgr::getSingleton();
	return mgr->keyHasJustBeenPressed(CT_KEYBOARD, keyboardMap(kc));
}

bool rs::Input::isKeyboardKeyJustReleased(Pasta::Key kc) {
	InputMgr* mgr = Pasta::InputMgr::getSingleton();
	return mgr->keyHasJustBeenReleased(CT_KEYBOARD, keyboardMap(kc));
}

bool rs::Input::isKeyDown(Pasta::ControllerType ct, Pasta::Key kc) {
	InputMgr* mgr = Pasta::InputMgr::getSingleton();
	return mgr->keyIsPressed(ct, keyboardMap(kc));
}

bool rs::Input::isKeyUp(Pasta::ControllerType ct, Pasta::Key kc) {
	InputMgr* mgr = Pasta::InputMgr::getSingleton();
	return !mgr->keyIsPressed(ct, keyboardMap(kc));
}


bool rs::Input::isKeyJustPressed(Pasta::ControllerType ct, Pasta::Key kc) {
	InputMgr* mgr = Pasta::InputMgr::getSingleton();
	return mgr->keyHasJustBeenPressed(ct, keyboardMap(kc));
}

bool rs::Input::isKeyJustReleased(Pasta::ControllerType ct, Pasta::Key kc) {
	InputMgr* mgr = Pasta::InputMgr::getSingleton();
	return mgr->keyHasJustBeenReleased(ct, keyboardMap(kc));
}

float rs::Input::getKeyValue(Pasta::ControllerType ct, Pasta::Key kc) {
	InputMgr* mgr = Pasta::InputMgr::getSingleton();
	return mgr->getKeyValue(ct, keyboardMap(kc));
}

bool rs::Input::isDirJustPressed_Down() {
	if (isKeyJustPressed(CT_KEYBOARD, KB_DOWN)) return true;
	if (isAnyPadJustPressed(PAD_DOWN)) return true;
	return false;
}

bool rs::Input::isDirJustPressed_Up() {
	if (isKeyJustPressed(CT_KEYBOARD, KB_UP)) return true;
	if (isAnyPadJustPressed(PAD_UP)) return true;
	return false;
}

bool rs::Input::isDirJustPressed_Left() {
	if (isKeyJustPressed(CT_KEYBOARD, KB_LEFT)) return true;
	if (isAnyPadJustPressed(PAD_LEFT)) return true;
	return false;
}

bool rs::Input::isDirJustPressed_Right() {
	if (isKeyJustPressed(CT_KEYBOARD, KB_RIGHT)) return true;
	if (isAnyPadJustPressed(PAD_RIGHT)) return true;
	return false;
}

bool rs::Input::isDirPressed_Down() {
	if (isKeyDown(CT_KEYBOARD, KB_DOWN)) return true;
	if (onAnyPadDown(PAD_DOWN)) return true;
	return false;
}

bool rs::Input::isDirPressed_Up() {
	if (isKeyDown(CT_KEYBOARD, KB_UP)) return true;
	if (onAnyPadDown(PAD_UP)) return true;
	return false;
}

bool rs::Input::isDirPressed_Left() {
	if (isKeyDown(CT_KEYBOARD, KB_LEFT)) return true;
	if (onAnyPadDown(PAD_LEFT)) return true;
	return false;
}

bool rs::Input::isDirPressed_Right() {
	if (isKeyDown(CT_KEYBOARD, KB_RIGHT)) return true;
	if (onAnyPadDown(PAD_RIGHT)) return true;
	return false;
}

bool rs::Input::isDirReleased_Down()
{
	if (isKeyJustPressed(CT_KEYBOARD, KB_DOWN)) return true;
	if (isAnyPadJustReleased(PAD_DOWN)) return true;
	return false;
}

bool rs::Input::isDirReleased_Up()
{
	if (isKeyJustPressed(CT_KEYBOARD, KB_UP)) return true;
	if (isAnyPadJustReleased(PAD_UP)) return true;
	return false;
}

bool rs::Input::isDirReleased_Left()
{
	if (isKeyJustPressed(CT_KEYBOARD, KB_LEFT)) return true;
	if (isAnyPadJustReleased(PAD_LEFT)) return true;
	return false;
}

bool rs::Input::isDirReleased_Right()
{
	if (isKeyJustPressed(CT_KEYBOARD, KB_RIGHT)) return true;
	if (isAnyPadJustReleased(PAD_RIGHT)) return true;
	return false;
}

bool rs::Input::onAnyPadDown(Pasta::Key k) {
	InputMgr* mgr = Pasta::InputMgr::getSingleton();
	for (unsigned int i = 0; i < CT_PAD_COUNT; i++)
		if (rs::Input::isKeyDown((Pasta::ControllerType)i, keyboardMap(k)))
			return true;
	return false;
}

bool rs::Input::isAnyPadJustPressed(Pasta::Key k) {
	InputMgr* mgr = Pasta::InputMgr::getSingleton();
	for (unsigned int i = 0; i < CT_PAD_COUNT; i++)
		if (rs::Input::isKeyJustPressed((Pasta::ControllerType)i, keyboardMap(k)))
			return true;
	return false;
}

bool rs::Input::isAnyPadJustReleased(Pasta::Key k) {
	InputMgr* mgr = Pasta::InputMgr::getSingleton();
	for (unsigned int i = 0; i < CT_PAD_COUNT; i++)
		if (isKeyJustPressed((Pasta::ControllerType)i, keyboardMap(k)))
			return true;
	return false;
}

float rs::Input::getWheelValue() {
	InputMgr* mgr = Pasta::InputMgr::getSingleton();
	float val = mgr->getKeyValue(CT_MOUSE, MOUSE_WHEEL);
	return val;
}