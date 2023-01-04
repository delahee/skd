//#include "utf8.h"
#include "stdafx.h"

#include <functional>

#include "2-fonts/FontResource.h"
#include "1-texts/TextMgr.h"
#include "1-input/InputMgr.h"

#include "4-menus/imgui.h"

#include "r2/InputField.hpp"
#include "rd/FontManager.hpp"

using namespace std;
using namespace Pasta;
using namespace r2;

#define SUPER Interact
r2::InputField::InputField(r2::InputConf & _conf, Node * parent) : conf(_conf), SUPER(0, 0, parent) {
	defaultValueColor = rd::ColorLib::get(0xcdcdcd, 1.0);

	rd::Font * fnt = _conf.font;
	float guessedHeight = fnt->getSize();

	Graphics * gfx = new Graphics(this);
	gfx->setGeomColor( conf.bgColor );
	gfx->drawQuad(0, 0, x + conf.width, y + guessedHeight + conf.topPadding * 2 + 1);

	bg = gfx;
	Node * localParent = this;

	localParent = mask = new r2::Scissor(Rect(0, 0, conf.width + 4, guessedHeight + conf.topPadding * 2 + 1), this);

	tf = new r2::Text(conf.font, conf.text, localParent);
	tf->setTextColor(conf.textColor);
	value = conf.text;
	syncVal();

	caret = r2::Graphics::rect(0, 0, 1, guessedHeight + conf.topPadding * 2 + 1, rd::ColorLib::toInt( conf.textColor ), 1.0, tf);
	caret->visible = false;
	
	trsDirty = true;

	createDelegate();
}

void r2::InputField::createDelegate() {
	onFocuses.push_back( std::bind( &InputField::_onFocus, this, std::placeholders::_1));
	onFocusLosts.push_back(std::bind(&InputField::_onFocusLost, this, std::placeholders::_1));

	onChars.push_back(std::bind(&InputField::_onChar, this, std::placeholders::_1));
	onKeyUps.push_back(std::bind(&InputField::_onKeyUp, this, std::placeholders::_1));
	onKeyDowns.push_back(std::bind(&InputField::_onKeyDown, this, std::placeholders::_1));
}

void r2::InputField::_onChar(rs::InputEvent & ev) {
	InputMgr * mgr = InputMgr::getSingleton();
	Scene * sc = getScene();
	if (!sc) return;
	if (sc->getCurrentFocus() != this) return;//not the focus
	bool vis = sc->computeNodeVisibility(this);
	if (!vis) return;//not really visible

	unsigned char c = ev.charCode;

	if (c < 25) {
		return;
	}

	//printf("2c- key: %d\n", c);

	ev.stopPropagation = true;

	handleKey(c);

	if (ev.charCode != 0 && value.size() < maxChar) {
		char lbl[8];
		memset(lbl, 0, 8);

		Pasta::TextMgr* mgr = TextMgr::getSingleton();

		int idx = 0;
		mgr->encodeUTF8(ev.charCode, lbl, &idx);
		mgr->encodeUTF8(0, lbl, &idx);
		insertAtCaret( std::string(&lbl[0]) );
		ev.stopPropagation = true;
	}
}

void r2::InputField::handleKey(unsigned char key) {
	//printf("key %c\n", key);
}

void r2::InputField::_onKeyUp(rs::InputEvent & ev) {
	Pasta::InputMgr * mgr = Pasta::InputMgr::getSingleton();
	Scene * sc = getScene();
	if (!sc) return;
	if (sc->getCurrentFocus() != this) return;//not the focus
	bool vis = sc->computeNodeVisibility(this);
	if (!vis) return;//not really visible

	//unsigned char c = mgr->getLocalCharOfKey(ev.native);
	//printf("2b- key: %d\n", c);

	if (ev.native == Key::KB_ESC) {
		blur(true);
		ev.stopPropagation = true;
		return;
	}

	if (ev.native == Key::KB_ENTER
		|| ev.native == Key::KB_NUMPAD_ENTER
		) {
		if (deferValidation) onChange(value);
		syncVal();
		sigEnter.trigger();
		ev.stopPropagation = true;

		blur();
		return;
	}
}

void r2::InputField::_onKeyDown(rs::InputEvent & ev) {
	InputMgr * mgr = InputMgr::getSingleton();
	Scene * sc = getScene();
	if (!sc) return;

	if (sc->getCurrentFocus() != this) return;//not the focus
	bool vis = sc->computeNodeVisibility(this);
	if (!vis) return;//not really visible

	//unsigned char c = mgr->getLocalCharOfKey(ev.native);
	//printf("2- key: %d\n", c);

	if (ev.native == Key::KB_V) {
		if (rs::Input::isKeyboardKeyDown(Key::KB_CTRL_LEFT)
			|| rs::Input::isKeyboardKeyDown(Key::KB_CTRL_RIGHT)) {
			//handle paste ?

			const char * t  = ImGui::GetClipboardText();

			insertAtCaret(string(t));
			ev.stopPropagation = true; return;
		}
	}

	if (ev.native == Key::KB_LEFT) {
		setCaretPos(caretPos - 1);
		ev.stopPropagation = true; return;
	}

	if (ev.native == Key::KB_RIGHT) {
		setCaretPos(caretPos + 1);
		ev.stopPropagation = true; return;
	}

	if (ev.native == Key::KB_HOME) {
		setCaretPos(0);
		ev.stopPropagation = true; return;
	}

	if (ev.native == Key::KB_END) {
		setCaretPos(tf->getNbElements());
		ev.stopPropagation = true; return;
	}

	if (ev.native == Key::KB_DEL) {
		if (value.size() > 0 && caretPos < value.size() ) {
			value.erase(caretPos,  1);
			if (!deferValidation) onChange(value);
			syncVal();
			setCaretPos(caretPos);
			ev.stopPropagation = true;
		}
		return;
	}

	if (ev.native == Key::KB_BACKSPACE) {
		if ( (caretPos > 0) && (caretPos <= value.size()) && (value.size() > 0) ) {
			value.erase(caretPos-1, 1);
			caretPos--;
			if (!deferValidation) onChange(value);
			syncVal();
			setCaretPos(caretPos);
			ev.stopPropagation = true;
			return;
		}
	}

	
}

void r2::InputField::onChange(std::string & newValue) {
	if (onChangeCbk) {
		onChangeCbk(newValue);
	}
}

void r2::InputField::_onFocus(rs::InputEvent & ev ) {
	int i = 0;
	if (isDefaultValue) {
		tf->setTextColor( conf.textColor );
		value = "";
		isDefaultValue = false;
		syncVal();
	}

	setCaretPos( value.size() );
	caret->visible = true;

	sigFocus.trigger();
}

void r2::InputField::_onFocusLost(rs::InputEvent & ev ) {
	caret->visible = false;
	if (value == defaultValue || value == "") {
		tf->setTextColor(defaultValueColor);
		isDefaultValue = true;
		value = defaultValue;
	}

	sigFocusLost.trigger();
}

r2::InputField::~InputField() {
	deleteAllChildren();
}

void r2::InputField::setDefaultValue(const std::string& value) {
	defaultValue = value;
	setAsDefault(defaultValueColor);
}

void r2::InputField::setValue(const std::string & t) {
	value = t;
	syncVal();
	setCaretPos(value.size());
}

void r2::InputField::update(double dt) {
	SUPER::update(dt);

	caretTimer += dt;
	if (caretTimer > 0.2) {
		caret->alpha = 1.0 - caret->alpha;
		caretTimer = fmodf(caretTimer, 0.1f);
	}
}

void r2::InputField::syncX() {
	if (conf.leftAlign)
		tf->x = conf.leftPadding;
	else
		tf->x = conf.width - tf->getSize().x;
	tf->y = conf.topPadding;
}

void r2::InputField::syncVal() {
	tf->setText(value);
	syncX();
}

void r2::InputField::setAsDefault( r::Color col ) {
	tf->setTextColor( defaultValueColor = col );
	isDefaultValue = true;
	defaultValue = value;
}

void r2::InputField::insertAtCaret(const std::string& str ) {
	if (isDefaultValue) {
		tf->setTextColor( conf.textColor );
		value = "";
		isDefaultValue = false;
	}

	int pos = caretPos;
	
	if (pos < 0) pos = 0;
	if (pos >= value.size()) pos = value.size();

	value = value.substr(0, pos) + str + value.substr(pos);
	syncVal();
	setCaretPos(pos + str.size() );
}

void r2::InputField::setCaretPos(int v) {
	if (v < 0) v = 0;
	BatchElem * e = tf->getElement(v);
	if(e)	
		caret->x = e->x - 1;
	else {
		e = tf->getElement(tf->getNbElements()-1);
		if (e)
			caret->x = e->x + e->width() + 1;
		else
			caret->x = 1;
	}
	caretPos = v;
}


#undef SUPER