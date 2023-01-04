#include "stdafx.h"

#include "../rs/all.hpp"
#include "TileAnim.hpp"
#include "TileAnimPlayer.hpp"
#include "Pools.hpp"

using namespace std;
using namespace rd;

TileAnim * TileAnimPlayer::getCurrentAnim() {
	if (stack.size() == 0)
		return nullptr;
	else
		return stack.at(0);
}

bool TileAnimPlayer::isFinished() {
	auto anm = getCurrentAnim();
	if (!anm)
		return true;
	return anm->plays <= 0;
}

void TileAnimPlayer::update(double dt) {
	if (needUpdates)
		_update(dt);
}

TileAnimPlayerExtension* rd::TileAnimPlayer::extend(){
	if (!ext) 
		ext = new TileAnimPlayerExtension(this);
	return ext;
}

TileAnimPlayer::TileAnimPlayer(IAnimated * _spr){
	spr = _spr;
 	int here = 0;
}

TileAnimPlayer& rd::TileAnimPlayer::operator=(const TileAnimPlayer& ot){
	needUpdates = ot.needUpdates;
	destroyed = false;
	isPlaying = ot.isPlaying;
	stack = rs::Std::map < rd::TileAnim*, rd::TileAnim* > (ot.stack, [](rd::TileAnim* a) {
		auto na = rd::Pools::anims.alloc();
		na->copy(*a);
		return na;
	});
	bus = ot.bus;

	//don't touch spr
	//IAnimated* spr = nullptr;
	frameRate = ot.frameRate;//default is taken from lib
	speed = ot.speed;
	onFrame = ot.onFrame;
	lib = ot.lib;
	return *this;
}

rd::TileAnimPlayer::~TileAnimPlayer() {
	if (ext) {
		delete ext;
		ext = nullptr;
	}
	for (auto anim : stack)
		rd::Pools::anims.free(anim);
	stack.clear();
}

TileAnimPlayer * TileAnimPlayer::chain(const char * id) {
	play(id, 1, true);
	return this;
}

TileAnimPlayer * TileAnimPlayer::chainAndLoop(const char * k) {
	chain(k)->loop();
	return this;
}

TileAnimPlayer * TileAnimPlayer::loop() {
	if (hasAnim())
		getLastAnim()->plays = 666666666;
	return this;
}

TileAnimPlayer * TileAnimPlayer::playAndLoop(const char * k) {
	return play(k)->loop();
}

TileAnimPlayer *  TileAnimPlayer::play(const char * group, int nbPlays, bool queueAnim ) {
	if (!isReady()) return this;

	lib = spr->getLib();
	if (!lib) 
		return this;

	TileGroup * g = lib->getGroup(group);
	if (!g) {
		#ifdef _DEBUG
		printf("no such group %s in lib %s...\n",group, lib->name.c_str());
		#endif
		return this;
	}

	if (g->anim.size() == 0)
		return this;

	if (!queueAnim && hasAnim())
		stop();

	TileAnim * a = rd::Pools::anims.alloc();
	a->libName = lib->name.c_str();
	a->reset(group, g,0, 0, nbPlays, false, 1.0);
	
	isPlaying = true;
	stack.push_back(a);
	startUpdates();

	if (!queueAnim)
		initCurrentAnim();

	return this;
}
	
int rd::TileAnimPlayer::getFrameNumber() {
	TileAnim* a = getCurrentAnim();
	if (!a) return 0; 
	if (!a->groupData) return 0; 
	return a->cursor;
}

int rd::TileAnimPlayer::getCursor() {
	TileAnim* a = getCurrentAnim();
	if (a) return a->cursor;
	return 0;
}

void rd::TileAnimPlayer::setFrame(int frame){
	TileAnim* a = getCurrentAnim();
	if (a) {
		int cursor = -1;
		for (int i = 0; i < a->groupData->anim.size();++i)
			if (frame == a->groupData->anim[i]) {
				cursor = i;
				break;
			}

		if (cursor >= 0) {
			spr->setFrame(frame);
			a->cursor = cursor;
			if (onFrame) onFrame(a->cursor);
		}
		else {
			//no sé?
		}
	}
}

void rd::TileAnimPlayer::setCursor(int cursor){
	TileAnim * a = getCurrentAnim();
	if (a) {
		a->cursor = clamp<int>(cursor, 0, a->nbFrames() - 1);
		spr->setFrame(a->cursor);
		if (onFrame) onFrame(a->cursor);
	}
	//else ?
}

double rd::TileAnimPlayer::getFinalAnimSpeed() {
	double libSpeed = lib ? lib->speed : 1.0;
	if (bus != nullptr)
		libSpeed *= bus->speed;
	return libSpeed * speed * getCurrentAnim()->speed;
}

bool rd::TileAnimPlayer::isReady(){
	return spr!=nullptr && spr->getLib();
}

void rd::TileAnimPlayer::unsync(){
	if (!getCurrentAnim())return;
	TileAnim & a = *getCurrentAnim();
	int nbFr = a.nbFrames() - 1;
	if(nbFr>0)
		a.cursor = rd::Rand::get().dice(0, nbFr);
}

void rd::TileAnimPlayer::dispose(){
	destroyed = true;
	if (!stack.size()) return;

	for (auto anim : stack)
		rd::Pools::anims.free(anim);

	stack.clear();
	bus = nullptr;
	if (ext)ext->reset();
}

void rd::TileAnimPlayer::reset()
{
	dispose();
	needUpdates = true;
	destroyed = false;
	isPlaying = false;
	speed = 1.0;
}

void rd::TileAnimPlayer::copy(TileAnimPlayer&toCopy)
{
	needUpdates = toCopy.needUpdates;
	destroyed = toCopy.destroyed;
	isPlaying = toCopy.isPlaying;

	//nope
	//spr = toCopy.spr;
	frameRate = toCopy.frameRate;//default is taken from lib
	speed = toCopy.speed;
	lib = toCopy.lib;

	for (TileAnim * ta : toCopy.stack) {
		TileAnim* nta = rd::Pools::anims.alloc();
		nta->copy(*ta);
		stack.push_back(nta);
	}
}

void rd::TileAnimPlayer::syncCursorToSpr(){
	auto a = getCurrentAnim();
	if (a) {
		a->cursor = clamp<int>(a->cursor, 0, a->nbFrames() - 1);
		spr->setFrame(a->cursor);
	}
}

float TileAnimPlayer::getDuration() {
	auto a = getCurrentAnim();
	//if no animation, assume it's a still frame ?
	if (!a) return 0.0f;
	int nb = a->nbFrames();
	return nb / frameRate / getFinalAnimSpeed();
}

void TileAnimPlayer::initCurrentAnim() {
	auto a = getCurrentAnim();
	spr->set(nullptr, a->groupName.c_str(), 0);
}

void TileAnimPlayer::_update(double dt){
	double libSpeed  = lib ? lib->speed : 1.0;
	auto cur = getCurrentAnim();
	if (cur != nullptr && !cur->paused && isPlaying ) {
		TileAnim & a = *cur;
		float rframe = dt * frameRate;
		float fspeed = getFinalAnimSpeed();
		a.curFrameCount += fspeed * rframe;

		while (a.curFrameCount > 1) {
			a.curFrameCount--;
			a.cursor++;

			if (a.cursor >= a.nbFrames()) {
				a.cursor = 0;
				a.plays--;
				if (a.plays <= 0) {

					if (ext) ext->onAnimEnd.trigger();

					if (a.onEnd != nullptr) {
						auto cb = a.onEnd;//copy is voluntary
						a.onEnd = nullptr;
						bool interruptAll = cb(this);
						if (interruptAll) {
							return;
						}
					}
					
					if (hasAnim())
						nextAnim();

					if (!hasAnim()) 
						break;
				}
				else {
					bool force = false;
					if (a.nbFrames() == 1)
						force = true;

					if (force || (spr->getFrame() != a.cursor)) {
						if (ext) ext->onAnimFrame.trigger();
						if (onFrame) onFrame(a.cursor);
						spr->setFrame(a.cursor);
					}
				}
			}
			else {
				if (spr->getFrame() != a.cursor) {
					if (ext) ext->onAnimFrame.trigger();
					if (onFrame) onFrame(a.cursor);
					spr->setFrame(a.cursor);
				}
			}
		}
	}

	if (!destroyed && !hasAnim())
		stopUpdates();
}

void TileAnimPlayer::stopUpdates() {
	needUpdates = false;
}

void TileAnimPlayer::startUpdates() {
	needUpdates = true;
}

void TileAnimPlayer::resume() {
	isPlaying = true;
}

void TileAnimPlayer::pause() {
	isPlaying = false;
}

void TileAnimPlayer::stop(){
	isPlaying = false;
	for(auto anim : stack)
		rd::Pools::anims.free(anim);
	stack.clear();
}

bool TileAnimPlayer::hasAnim(){
	return stack.size()>0;
}

void rd::TileAnimPlayer::nextAnim()
{
	auto cur = getCurrentAnim();

	//dispose after play ?

	{
		TileAnim * anm = stack.at(0);
		anm->clear();
		rd::Pools::anims.free(anm);
		stack.erase(stack.begin());
	}

	if (stack.size() == 0) {
		// End
		stop();
	}
	else {
		// Init next
		initCurrentAnim();
	}
}

void rd::TileAnimPlayer::addOnEnd(std::function<bool(rd::TileAnimPlayer*)> fun) {
	if (stack.size() == 0)
		return;
	std::function<bool(rd::TileAnimPlayer*)> old = getLastAnim()->onEnd;//copy func
	getLastAnim()->onEnd = [old,fun](auto pl) {
		bool inter0 = (old) ? old(pl) : false;//aggreg results
		bool inter1 = fun(pl);
		return inter0 || inter1;
	};
}

void rd::TileAnimPlayer::setOnEnd( std::function<bool(rd::TileAnimPlayer *)> fun){
	if (stack.size() == 0)
		return;
	
	getLastAnim()->onEnd = fun;
}

TileAnim * TileAnimPlayer::getLastAnim(){
	if (stack.size() == 0)
		return nullptr;
	return stack.at(stack.size()-1);
}

void TileAnimPlayer::im() {
	using namespace ImGui;

	auto cur = getCurrentAnim();
	if (!cur) {
		ImGui::Text("no animation...");
		return;
	}
	Text("%d / %d", cur->cursor + 1, cur->nbFrames());
	Value("Duration", getDuration());
	Value("Nb Plays", cur->plays);

	if (!isPlaying) {
		if (Button(ICON_MD_PLAY_ARROW)) 
			resume();
	}
	else {
		if (Button(ICON_MD_PAUSE)) 
			pause();
	}
	SameLine();
	if (Button(ICON_MD_SKIP_PREVIOUS)) {
		auto cur = getCurrentAnim();
		pause();
		cur->cursor--;
		syncCursorToSpr();
	}
	SameLine();
	if (Button(ICON_MD_SKIP_NEXT)) {
		pause();
		cur->cursor++;
		syncCursorToSpr();
	}
	SameLine();
	if (Button(ICON_MD_FAST_REWIND)) {
		cur->cursor = 0;
		pause();
		syncCursorToSpr();

	}
	SameLine();
	if (Button(ICON_MD_FAST_FORWARD)) {
		cur->cursor = cur->nbFrames()-1;
		pause();
		syncCursorToSpr();
	}

	
	if (isPlaying) {
		SameLine();
		Text("Playing");
	}
}

void rd::TileAnimPlayerExtension::reset()
{
	onAnimFrame.clear();
	onAnimEnd.clear();
}
