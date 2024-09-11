#include "stdafx.h"

#include <cmath>

#include "../rs/Sys.hpp"
#include "../rs/ITweenable.hpp"
#include "../rs/Pool.hpp"
#include "rplatform/CrossPlatform.hpp"

#include "Tween.hpp"

#ifndef PASTA_FINAL
	//#define DEBUG_TWEEN
#endif

using namespace std;

using namespace rd;
using namespace rs;
int Tween::GUID = 0;

#define SUPER rd::Agent
Tween::Tween(){

}

static std::string tv_to_string(TVar tv) {
	switch (tv) {
	case VNone:			return "Vnone";
	case VX:			return "VX";
	case VY:			return "VY";
	case VZ:			return "VZ";
	case VScaleX:		return "VScaleX";
	case VScaleY:		return "VScaleY";
	case VAlpha:		return "VAlpha";
	case VRotation:		return "VRotation";
	case VR:			return "VR";
	case VG:			return "VG";
	case VB:			return "VB";
	case VA:			return "VA";
	case VScale:		return "VScale";
	case VWidth:		return "VWidth";
	case VHeight:		return "VHeight";
	case VCustom0:		return "VCustom0";
	case VCustom1:		return "VCustom1";
	case VCustom2:		return "VCustom2";
	case VCustom3:		return "VCustom3";
	case VCustom4:		return "VCustom4";
	case VCustom5:		return "VCustom5";
	case VCustom6:		return "VCustom6";
	case VCustom7:		return "VCustom7";
	default:			return "unknown";
	}
}

void rd::Tween::reset(ITweenable * _parent, TVar _vname, double _dur,  double _from, double _to, TType _type, int _nbPlays, std::function<double(double)> _interpolate){
	parent = _parent;

#ifdef DEBUG_TWEEN
	auto asSpr = dynamic_cast<r2::Node*>(parent);
	if (asSpr) {
		targetUID = asSpr->uid;
		targetName = asSpr->name;
	}
	else {
		targetUID = 0;
		targetName.clear();
	}
#endif

	vname = _vname;
	n = 0.0;
	time = 0;
	duration = _dur;
	from = _from;
	to = _to;
	type = _type;
	nbPlays = _nbPlays;
	interpolate = _interpolate;
	autoReverse = false;

	afterDelay = nullptr;
	onUpdate = nullptr;
	onUpdateT = nullptr;
	onEnd = nullptr;
	pixelMode = false;

	uid = GUID++;
}

void rd::Tween::im() {
	using namespace ImGui;
	double dist = to - from;
	double ln = time / duration;
	if (duration == 0.0)
		ln = 0.0;
	double val = from + interpolate(ln) * dist;
	Text(tv_to_string(vname));
	SameLine(); 
	Text("%.2f", val);
	SameLine();
	if (parent) 
		Text("Parent : 0x%x", parent);
	Text("To %.2f", to);
	Text("From %.2f", from);
	Text("Dur %.2f ms",  (man->FPS * speed)*1000.0f);
	Value("Auto Reverse", autoReverse);
	Value("Nb Plays", this->nbPlays);
	Value("Target Value",targetUID);
	Value("Target Name",targetName);

	Text("n %f / ln %.2f", n,ln);
	if( delayMs>0)
		Text("Delay %.2f ms", delayMs);
	
	NewLine();

}

void rd::Tween::clear() {
	n = 0.0;
	duration = 0.0;
	speed = 0.0;
	nbPlays = 0;
	from = 0.0;
	to = 0.0;
	vname = VNone;
	autoReverse = false;
	interpolate = nullptr;
	parent = nullptr;

	afterDelay = nullptr;
	onUpdate = nullptr;
	onUpdateT = nullptr;
	onEnd = nullptr;
	userData = nullptr;
}

float rd::Tweener::normalizeAngle(float a) {
	//don't use modulo at it can give sign oscillating stuff which is bad for interps
	float pi = PASTA_PI;
	while (a > pi)
		a -= pi * 2;
	while (a < -pi)
		a += pi * 2;
	return a;
}

Tweener::Tweener() {
	deleteSelf = false;
	setName("Tweener");
}

rd::Tweener::~Tweener(){
	onDispose();
}

void rd::Tweener::onDispose() {
	clear();
}

bool Tweener::exists(ITweenable * p, rs::TVar v) {
	for (auto t = delayList.begin(); t != delayList.end(); t++)
		if ((*t)->parent == p && (*t)->vname == v)
			return true;
	for ( auto t = tList.begin(); t != tList.end(); t++)
		if ((*t)->parent == p && (*t)->vname == v)
			return true;
	return false;
}

rd::Tween* rd::Tweener::get(rs::ITweenable* p, rs::TVar v) {
	for (auto t = delayList.begin(); t != delayList.end(); t++)
		if ((*t)->parent == p && (*t)->vname == v)
			return *t;
	for (auto t = tList.begin(); t != tList.end(); t++)
		if ((*t)->parent == p && (*t)->vname == v)
			return *t;
	return nullptr;
}

void Tweener::forceTerminateTween(Tween * t) {
	auto pos = std::find(tList.begin(), tList.end(), t);
	auto posDelay = std::find(delayList.begin(), delayList.end(), t);
	
	//was already free
	if (pos == tList.end() && posDelay == delayList.end())
		return;

	if (pos != tList.end())
		tList.erase(pos);

	if (posDelay != delayList.end())
		delayList.erase(posDelay);
	
	t->clear();
	
	pool.release(t);
}

void rd::Tweener::terminateAllTween(ITweenable * p) {
	for (int i = tList.size() - 1; i >= 0; i--) 
		if (tList[i]->parent == p) 
			terminateTween(tList[i], false);

	for (int i = delayList.size() - 1; i >= 0; i--) 
		if (delayList[i]->parent == p) 
			terminateTween(delayList[i], false);
}

void rd::Tweener::terminateTween(rs::ITweenable* t, TVar tv) {
	for (int i = tList.size() - 1; i >= 0; i--) 
		if ((tList[i]->parent == t) && (tList[i]->vname == tv)) 
			terminateTween(tList[i], false);

	for (int i = delayList.size() - 1; i >= 0; i--) 
		if ((delayList[i]->parent == t) && (delayList[i]->vname == tv))
			terminateTween(delayList[i], false);
}

void rd::Tweener::killAllTween(ITweenable* p) {
	for (int i = tList.size() - 1; i >= 0; i--)
		if (tList[i]->parent == p)
			forceTerminateTween(tList[i]);

	for (int i = delayList.size() - 1; i >= 0; i--)
		if (delayList[i]->parent == p)
			forceTerminateTween(delayList[i]);
}

rd::Tween* Tweener::createColorHSV(rs::ITweenable* parent, r::Color to, rd::TType tp, double duration_ms, bool allowDup ){
	auto main = create(parent, TVar::VCustom0, 0.0, tp, duration_ms, allowDup);
	r::Color from( 
		(float)parent->getValue(TVar::VR),
		(float)parent->getValue(TVar::VG),
		(float)parent->getValue(TVar::VB),
		(float)parent->getValue(TVar::VA)
	);
	main->onUpdateT = [from,to](auto spr,auto t) {
		r::Color interp = r::Color::lerpHSV(from, to, t);
		spr->setValue(TVar::VR, interp.r);
		spr->setValue(TVar::VG, interp.g);
		spr->setValue(TVar::VB, interp.b);
		spr->setValue(TVar::VA, interp.a);
	};
	return main;
}

rd::Tween* Tweener::createColorLinear(rs::ITweenable* parent, r::Color to, rd::TType tp, double duration_ms, bool allowDup ){
	auto main = 
		create(parent, TVar::VR, to.r, tp, duration_ms, allowDup);
	create(parent, TVar::VG, to.g, tp, duration_ms, allowDup);
	create(parent, TVar::VB, to.b, tp, duration_ms, allowDup);
	create(parent, TVar::VA, to.a, tp, duration_ms, allowDup);
	return main;
}

Str s_dbgBreakOnName = "body";
rd::Tween * Tweener::create(ITweenable * parent, rs::TVar varName, double to, TType tp, double duration_ms, bool allowDup ){
	if (parent == nullptr) return nullptr;
	if (duration_ms == -1.0) duration_ms = DEFAULT_DURATION;

#if 0
	if (s_dbgBreakOnName.length()) {
		
		auto asNode = dynamic_cast<r2::Node*>(parent);
		if (asNode) {
			traceNode("creating tw on parent",asNode);
		}
		if (asNode && parent && asNode->name == s_dbgBreakOnName) {
			__debugbreak();
		}
	}
#endif

	int iter = tList.size()-1;
	Tween ** data = tList.data();

	if(!allowDup)
	while(data && (iter >=0)) {
		Tween * mt = data[iter];
		if ((mt->parent == parent) && (mt->vname == varName))
			forceTerminateTween(mt);
		iter--;
	}

	double val = parent->getValue(varName);
	auto tw = pool.alloc();
	
	//hmmmm this seems like a mistake to normalize the valuated angle
	//if (varName == TVar::VRotation) 
	//	val = to - normalizeAngle(to - val);

	if (duration_ms == 0.0)
		duration_ms = 1e-6;
	tw->reset(parent, varName, duration_ms * 0.001, val, to, tp, 1, interpolators[int(tp)]);
	tw->delayMs = 0.0;
	
	tw->man = this;
	tList.push_back(tw);

	if( tw->uid == 181 || tw->uid == 182){
		int here = 0;
	}
#if 0 
	printf("tw created: %d dur:%f\n", varName, duration_ms);
#endif
	return tw;
}

Tween * rd::Tweener::createFromTo(rs::ITweenable * parent, rs::TVar varName, double from, double to, rd::TType tp, double duration_ms,bool allowDup) {
	Tween * p = create(parent, varName, to, tp, duration_ms, allowDup);
	if (varName == TVar::VRotation) {
		from = to - normalizeAngle(to - from);
	}
	p->from = from;
	return p;
}

rd::Tween* rd::Tweener::createPtr(rs::ITweenable* parent, double* ptr, double to, rd::TType tp, double duration_ms, bool allowDup) {
	Tween* p = createFromTo(parent, TVar::VPointer, 0 , to, TType::TLinear, duration_ms, allowDup);
	double from = *ptr;
	p->onUpdateT = [=](auto spr,auto t) {
		*ptr = from + (to-from) * rd::Tweener::interpolators[(int)tp](t);
	};
	return p;
}

rd::Tween* rd::Tweener::createPtr(rs::ITweenable* parent, float* ptr, float to, rd::TType tp, double duration_ms, bool allowDup) {
	Tween* p = createFromTo(parent, TVar::VPointer, 0, to, TType::TLinear, duration_ms, allowDup);
	float from = *ptr;
	p->onUpdateT = [=](auto spr, auto t) {
		*ptr = (float)(from + (to - from) * rd::Tweener::interpolators[(int)tp](t));
	};
	return p;
}

inline std::function<double(double)> Tweener::getInterpolateFunction(rd::TType type) {
	switch (type) {
		case TType::TLinear:		return [](double t) { return t; };
		case TType::TEase:			return [](double t) { return bezier(t, 0, 0, 1, 1);				};
		case TType::TEaseIn:		return [](double t) { return bezier(t, 0, 0, 0.5, 1);			};
		case TType::TEaseOut:		return [](double t) { return bezier(t, 0, 0.5, 1, 1);			};
		case TType::TBurn:			return [](double t) { return bezier(t, 0, 1, 0, 1);				};
		case TType::TBurnIn:		return [](double t) { return bezier(t, 0, 1, 1, 1);				};
		case TType::TBurnOut:		return [](double t) { return bezier(t, 0, 0, 0, 1);				};
		case TType::TZigZag:		return [](double t) { return bezier(t, 0, 2.5, -1.5, 1);		};
		case TType::TLoop:			return [](double t) { return bezier(t, 0, 1.33, 1.33, 0);		};
		case TType::TCos:			return [](double t) { return cos(t * PASTA_PI * 2);				};
		case TType::TSin:			return [](double t) { return sin(t * PASTA_PI * 2);				};
		case TType::TLoopEaseIn:	return [](double t) { return bezier(t, 0, 0, 2.25, 0);			};
		case TType::TLoopEaseOut:	return [](double t) { return bezier(t, 0, 2.25, 0, 0);			};
		case TType::TJump:			return [](double t) { return bezier(t, 0, 2, 2.79, 1);			};
		case TType::TShake:			return [](double t) { return fastPow3(sin(t * PASTA_PI * 16));	};
		case TType::TBounceOut:		return [](double t) {
			if (t < ( 1 / 2.75 )) {
				return 7.5625 * t * t;
			}
			else if (t < ( 2 / 2.75 )) {
				return 7.5625 * ( t -= ( 1.5 / 2.75 ) ) * t + 0.75;
			}
			else if (t < ( 2.5 / 2.75 )) {
				return 7.5625 * ( t -= ( 2.25 / 2.75 ) ) * t + 0.9375;
			}
			else {
				return 7.5625 * ( t -= ( 2.625 / 2.75 ) ) * t + 0.984375;
			}	
		};
		case TType::TElasticEnd:	return [](double t) { return bezier(t, 0, 0.7, 1.5, 1);		};
		case TType::TQuad:			return [](double t) { return t * t;		};
		case TType::TCubic:			return [](double t) { return t * t * t;		};
		default:					return [](double t) { return t; };		  

	}
}

std::function<double(double)> Tweener::interpolators[(int)rd::TType::TCount] = {
	Tweener::getInterpolateFunction(rd::TType::TLinear),
	Tweener::getInterpolateFunction(rd::TType::TLoop),
	Tweener::getInterpolateFunction(rd::TType::TLoopEaseIn),
	Tweener::getInterpolateFunction(rd::TType::TLoopEaseOut),
	Tweener::getInterpolateFunction(rd::TType::TCos),
	Tweener::getInterpolateFunction(rd::TType::TSin),
	Tweener::getInterpolateFunction(rd::TType::TEase),
	Tweener::getInterpolateFunction(rd::TType::TEaseIn),
	Tweener::getInterpolateFunction(rd::TType::TEaseOut),
	Tweener::getInterpolateFunction(rd::TType::TBurn),
	Tweener::getInterpolateFunction(rd::TType::TBurnIn),
	Tweener::getInterpolateFunction(rd::TType::TBurnOut),
	Tweener::getInterpolateFunction(rd::TType::TZigZag),
	Tweener::getInterpolateFunction(rd::TType::TJump),
	Tweener::getInterpolateFunction(rd::TType::TBounceOut),
	Tweener::getInterpolateFunction(rd::TType::TShake),
	Tweener::getInterpolateFunction(rd::TType::TElasticEnd),
	Tweener::getInterpolateFunction(rd::TType::TQuad),
	Tweener::getInterpolateFunction(rd::TType::TCubic),
};

void rd::Tweener::onUpdate(Tween * t, double n ) {
	if (t->onUpdate != nullptr) 	t->onUpdate(t->parent);
	if (t->onUpdateT != nullptr)	t->onUpdateT(t->parent, n);
}

void rd::Tweener::onEnd( Tween * t) {
	if (t->onEnd != nullptr)
		t->onEnd(t->parent);
}

Tween * rd::Tweener::delay(
	double delay_ms, rs::ITweenable * parent, TVar varName, double to, TType tp, double duration_ms,bool allowDup) {
	Tween * p = create(parent, varName, to, tp, duration_ms, allowDup);
	
	//remove actual tween and push to delay
	auto pos = std::find(tList.begin(), tList.end(), p);
	if(pos!=tList.end())
		tList.erase(pos);

	delayList.push_back(p);//finish pair
	p->delayMs = delay_ms;
	return p;
}

Tween * rd::Tweener::delayFromTo(double delay_ms, rs::ITweenable * parent, rs::TVar varName, double from, double to, TType tp, double duration_ms, bool allowDup) {
	Tween * p = createFromTo(parent, varName, from, to, tp, duration_ms,allowDup);

	auto pos = std::find(tList.begin(), tList.end(), p);
	if (pos != tList.end())
		tList.erase(pos);

	delayList.push_back(p);//finish pair
	p->delayMs = delay_ms;
	return p;
}

void rd::Tweener::terminateTween(Tween * t, bool fl_allowLoop ) {
	double val = t->from + (t->to - t->from) * t->interpolate(1.0);
	t->parent->setValue( t->vname, val );
	onUpdate(t, 1);

	auto ouid = t->uid;

	onEnd(t);//if onend triggered a new tween...and reentered with the same

	if (ouid == t->uid) {
		if (fl_allowLoop && (t->nbPlays == -1 || t->nbPlays>1)) {
			if (t->nbPlays != -1)
				t->nbPlays--;
			t->time = 0;
			if (t->autoReverse) std::swap(t->from, t->to);
		}
		else 
			forceTerminateTween(t);
	}
}

void rd::Tweener::update(double dt){
	SUPER::update(dt);
	fr++;
	if (isLoadingFrame) {
		dt = 0.0001;
		isLoadingFrame = false;
	}

	if (bus)
		dt *= bus->speed;

	double deltaMs = dt * 1000.0;

	if (delayList.size() > 0) {
		int iter = delayList.size()-1;
		Tween ** data = delayList.data();
		while (data && (iter >= 0) ) {
			Tween * t = data[iter];
			t->delayMs -= deltaMs;
			if (t->delayMs <= 0.0) {
				delayList.erase(std::find(delayList.begin(),delayList.end(),t));
				if (t->afterDelay) t->afterDelay(t->parent);

				//update value for chains
				t->from = t->parent->getValue(t->vname);

				tList.push_back(t);
			}
			iter--;
		}
	}
	
	if (tList.size() > 0) {
		//auto end = ;
		int idx = tList.size()-1;
		while( idx >= 0 ){
			if(idx >= tList.size()){
				idx--;
				continue;
			}
			Tween * t = tList[idx];
			double dist = t->to - t->from;
			double ln = (t->time + dt) / t->duration;
			t->time += dt;
			t->n = t->interpolate(ln);
			if (ln<1) {
				double val = t->from + t->n*dist;

#ifdef DEBUG_TWEEN // ensure sprite consistency, this is a large cause of issues coupled with pools
				auto asNode = dynamic_cast<r2::Node*>(t->parent);
				if (asNode && asNode->uid != t->targetUID) {
					trace("tween uid ", t->targetUID);
					trace("tween vs  ", asNode->uid);
					traceNode("tween error on node : ",asNode);
					trace("name was"s + t->targetName.c_str());
					rplatform::debugbreak();
				}
#endif
				if(!t->pixelMode)
					t->parent->setValue(t->vname,val);
				else 
					t->parent->setValue(t->vname, std::round(val));
				onUpdate(t, ln);
#if 0
				printf("tw update: v:%d from:%f to:%f ln:%f \n", t->vname, t->from, t->to, t->ln);
#endif
			}
			else { // fini !
				terminateTween(t, true);
			}
			idx--;
		}
	}
}

rd::Tween::~Tween(){
	//traceTween("~", this);
}

void rd::Tweener::clear(){
	for (Tween * t : delayList) pool.release(t);
	for (Tween * t : tList) pool.release(t);

	delayList.clear();
	tList.clear();
}
bool rd::Tweener::im(){
	using namespace ImGui;
	if (!bus)
		Text("No linked bus");
	else
		bus->im();
	if (ImGui::TreeNodeEx("delayed",ImGuiTreeNodeFlags_DefaultOpen)) {
		for (Tween* t : delayList) t->im();
		TreePop();
	}
	if (ImGui::TreeNodeEx("actives", ImGuiTreeNodeFlags_DefaultOpen)) {
		for (Tween* t : tList) t->im();
		TreePop();
	}
	return false;
}
#undef SUPER

void rd::traceTween(const char* prefix, rd::Tween* obj){
#ifndef PASTA_FINAL
	if (obj) {
		Str256f msg("traceTween: %s uid:%ld targetUID:%ld targetName:%s addr:0x%x", prefix, obj->uid, obj->targetUID, obj->targetName.c_str(), obj);
		trace(msg);
	}
#endif 
}
