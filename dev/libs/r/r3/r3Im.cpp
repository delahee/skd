#include "stdafx.h"
#include "r3Im.hpp"
#include "r2/im/HierarchyExplorer.hpp"
#include "r2/ext/ExtGraphics.hpp"

static r::Color COL_AXIS_FORWARD = r::Color(1.0f, 0.0f, 0.0f, 1.0f);
static r::Color COL_AXIS_LEFT = r::Color(0.0f, 1.0f, 0.0f, 1.0f);
static r::Color COL_AXIS_RIGHT = r::Color(1.0f, 1.0f, 0.0f, 1.0f);
static r::Color COL_AXIS_UP = r::Color(0x5175FA);

std::function<r::vec3(const r::vec3&)> r3::im::transform;
std::function<r::vec3(const r::vec3&)> r3::im::untransform;

static r2::Scene* getMainScene() {
	auto he = r2::im::HierarchyExplorer::me;
	if (!he)
		return 0;

	r2::Scene* mainScene = 0;
	for (auto sc : he->scs) //let's trust the user
		if (sc->hasTag("world"))
			return sc;
	return 0;
}

r2::Graphics* r3::im::gizmo(const r::vec3& pos){
	//comes in iso space
	//draws in pixel space 
	r2::Scene* sc = getMainScene();
	if (!sc)
		return 0;
	if (!transform)
		return 0;
	r2::Graphics* g = rd::Pools::graphics.alloc();
	g->nodeFlags |= NF_UTILITY | NF_EDITOR_PROTECT;
	g->blendmode = r::TransparencyType::TT_OPAQUE;

	//this might prove much more accurate than internal guess
	bool useListenerOrigins = true;

	//draw a gizmo
	vec3 center = pos;
	vec3 up = vec3(0,0,10); 
	if(useListenerOrigins) up = rd::AudioMan::get().listenerUp;
	vec3 fwd = vec3(0,0,10);
	if (useListenerOrigins) fwd = rd::AudioMan::get().listenerFwd;

	r::Vector3 left = fwd.cross(up);

	up = (up * 2) / sc->cameraScale.x;
	fwd /= sc->cameraScale.x;
	left /= sc->cameraScale.x;

	Vector3 c = transform(center);
	Vector3 cu = transform(center + up);
	Vector3 cf = transform(center + fwd);
	Vector3 cl = transform(center + left);

	float thiccness = 4 / sc->cameraScale.x;
	r2::ext::drawLine(g, c, cu, COL_AXIS_UP, COL_AXIS_UP.lighten(0.1f), thiccness);
	r2::ext::drawLine(g, c, cf, COL_AXIS_FORWARD, COL_AXIS_FORWARD.lighten(0.1f), thiccness);
	r2::ext::drawLine(g, c, cl, COL_AXIS_LEFT, COL_AXIS_LEFT.lighten(0.1f), thiccness);

	if (sc)
		sc->addChild(g);

	r2::Im::depletedNodes.push_back(g);
	return g;

}
