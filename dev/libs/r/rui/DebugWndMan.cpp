#include "stdafx.h"
#include "DebugWndMan.hpp"
#if PASTA_WIN
#include "1-graphics/HWNDWindowMgr.h"
#endif

using namespace std;
using namespace rui;
using namespace rs;

DebugWndMan::DebugWndMan(rd::AgentList* al) :rd::Agent(al){
	name = "DebugWndMan";
	resos.push_back(Vector2i(480, 360));//4/3
	//resos.push_back(Vector2i(576, 360));//16/10
	resos.push_back(Vector2i(640, 360));//16/9
	//resos.push_back(Vector2i(840, 360));//21/9
	//resos.push_back(Vector2i(1280, 360));//32/9
	resos.push_back(Vector2i(720 * 16 / 9, 720));//HD Ready
	//resos.push_back(Vector2i(720 * 16 / 10, 720));//HD Ready
	resos.push_back(Vector2i(1080 * 16 / 9, 1080));//FULL HD
}

void DebugWndMan::update(double dt) {
	rd::Agent::update(dt);

#if PASTA_WIN
	auto s = Pasta::HWNDWindowMgr::getSingleton();
	bool changed = false;

	if ((rs::Input::isKeyboardKeyDown(Key::KB_CTRL_LEFT) || rs::Input::isKeyboardKeyDown(Key::KB_CTRL_RIGHT))
		&& rs::Input::isKeyboardKeyJustPressed(Key::KB_LEFT)) {
		if (cur <= 0)
			cur = 0;
		else
			cur--;
		changed = true;
	}

	if ((rs::Input::isKeyboardKeyDown(Key::KB_CTRL_RIGHT) || rs::Input::isKeyboardKeyDown(Key::KB_CTRL_LEFT))
		&& rs::Input::isKeyboardKeyJustPressed(Key::KB_RIGHT)) {
		if (cur >= resos.size()-1)
			cur = resos.size()-1;
		else
			cur++;
		changed=true;
	}

	if ((rs::Input::isKeyboardKeyDown(Key::KB_ALT_RIGHT) || rs::Input::isKeyboardKeyDown(Key::KB_ALT_LEFT))
		&& rs::Input::isKeyboardKeyJustPressed(Key::KB_ENTER)) {
		auto sz = s->getWindowSize(0);
		if (!s->IsFullscreen())
			s->enterFullScreen(0, sz.x,sz.y, true);
		else 
			s->exitFullScreen(0, sz.x, sz.y);
		changed = true;
	}

	if (changed) {
		auto e = (cur < 0) ? s->getWindowSize(0) : resos[cur];
		RECT r = {};
		r.top = r.left = 0;
		r.right = e.x;
		r.bottom = e.y;
		s->changeWindowSizeAt(0, r);
		auto ratio = [=]() -> std::string {
			float r = e.x / e.y;
			if (r::Math::approximatelyEqual(16.0f / 9.0f, 1e-3f))
				return "16/9";
			if (r::Math::approximatelyEqual(4.0f/3, 1e-3f))
				return "4/3";
			return std::to_string(r);
		};
		cout << "** changed resolution to " << to_string(e.x) << "x" << to_string(e.y) << " ratio = "+ ratio() +"\n";
	}
#endif
}