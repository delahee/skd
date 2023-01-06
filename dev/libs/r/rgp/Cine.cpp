#include "stdafx.h"
#include "Cine.hpp"


using namespace rgp;


void rgp::CineController::add(std::function<void(void)> f) { 
	auto an = new AnonCineStep(f);
	steps.push_back(an); 
}

void rgp::CineController::add(std::function<void(AnonCineStep*)> f) {
	auto an = new AnonCineStep(f);
	steps.push_back(an);
}

void rgp::CineController::im() {
	using namespace ImGui;
	Value("name", name);
	Value("cursor", cursor);
	int idx = 0;
	for(auto s : steps){
		r::Color col = r::Color::Grey;
		if (cursor == idx) {
			Text(ICON_MD_ARROW_FORWARD);
			SameLine();
			col = r::Color::Orange;
		}
		if (s->isFinished()) col = r::Color::AcidGreen;
		PushTextColor(col);
		Text(s->name);
		PopTextColor();
		idx++;
	}
}

void rgp::CineController::addInstant(CineStep* s) {
	steps.insert(steps.begin()+cursor+1,s);
}

bool CineController::start() {
	onStart();
	cursor = 0;
	if (!steps.size())
		return false;
	steps[cursor]->init(this);
	return true;
}
void rgp::CineController::pause() {
	speed = 0;
}

void CineController::update(double dt) {
	if (cursor < 0)
		return;
	if (cursor >= steps.size())
		return;
	auto cur = steps[cursor];

	//may add an instant step
	double ldt = dt * speed;

	if(ldt)
		cur->update(ldt);

	if (cur->isFinished()) {
		cur->dispose();
		cursor++;
		if (cursor >= steps.size())
			onEnd();
		else {
			steps[cursor]->init(this);
		}
	}
}

/////////////////////////////
/// Cine step section  //////
/////////////////////////////

rgp::CineStep::CineStep() {
	name = strrchr(__FUNCTION__,':')+1;
}


CineStep* rgp::CineController::waitForSeconds(double tSec) {
	CineStep* step = new WaitForSeconds(tSec);
	add(step);
	return step;
}

CineStep* rgp::CineController::instantWaitForSeconds(double tSec) {
	CineStep* step = new WaitForSeconds(tSec);
	addInstant(step);
	return step;
}

CineStep* rgp::CineController::waitForMs(double tMSec) {
	return waitForSeconds(tMSec / 1000.0f);
}

rgp::AnonCineStep::AnonCineStep(std::function<void(void)> f) {
	proc = [f](auto) {
		f();
	};
	name = strrchr(__FUNCTION__, ':')+1;
}

rgp::AnonCineStep::AnonCineStep(std::function<void(AnonCineStep*)> f) {
	proc = [f](auto lthis) {
		f(lthis);
	};
	name = strrchr(__FUNCTION__, ':')+1;
}

void rgp::AnonCineStep::update(double dt) {
	proc(this);
	if (autoFinish)
		finished = true;
}

