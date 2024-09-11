#include "stdafx.h"
#include "ClickHoldDetector.hpp"

rui::ClickHoldDetector::ClickHoldDetector(rui::Button* _bt) : bt(_bt) {
	setName("ClickHoldDetector");
	sigsHdlMDown = bt->sigOnMouseButtonDown.add([this]() {
		holding = true;
	});
	sigsHdlOut = bt->sigOnOut.add([this]() {
		holding = false;
	});
	sigsHdlBlur = bt->sigOnBlur.add([this]() {
		holding = false;
	});
	sigsHdlMUp = bt->sigOnMouseButtonUp.add([this]() {
		holding = false;
	});
	bt->al.add(this);
	deleteSelf = false;
}

void rui::ClickHoldDetector::update(double dt) {
	rd::Agent::update(dt);
	if (!bt->isEnabled())
		holding = false;
	if (!bt->isFocused())
		holding = false;
	if (holding && onHold)
		onHold();
}

void rui::ClickHoldDetector::dispose(){
	rd::Agent::dispose();
	if(sigsHdlClick)	bt->sigOnClick.remove(sigsHdlClick);
	if(sigsHdlBlur)		bt->sigOnBlur.remove(sigsHdlBlur);
	if(sigsHdlOut)		bt->sigOnOut.remove(sigsHdlOut);
	if(sigsHdlMDown)	bt->sigOnMouseButtonDown.remove(sigsHdlMDown);
	if(sigsHdlMUp)		bt->sigOnMouseButtonUp.remove(sigsHdlMUp);

	sigsHdlClick = 0;
	sigsHdlBlur = 0;
	sigsHdlOut = 0;
	sigsHdlMDown = 0;
	sigsHdlMUp = 0;
}
