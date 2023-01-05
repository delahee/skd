#pragma  once
#include "rd/Agent.hpp"
#include "rd/ext/Interp.hpp"
class Game;

struct Path : rd::Agent {
	Game* g = 0;

	typedef rd::Agent Super;
	Path(rd::AgentList* al) : Super(al) {

	};

	r2::Bitmap* cursor = 0;
	float					progress = 0;

	rd::ext::CurveC2		data;

	void add(const r::Vector2& p) { data.data.push_back(p); };
	void update(double dt);
	void reflectProgress(r2::Bitmap* c, float p);
	void debugDraw();
};