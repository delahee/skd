#include "stdafx.h"

#include "TileAnim.hpp"
#include "TileAnimPlayer.hpp"

using namespace std;
using namespace rd;

void rd::AnimBus::update() {
	finalSpeed = speed;
	if (parent) {
		parent->update();
		finalSpeed *= parent->finalSpeed;
	}
	if (finalSpeed > maxSpeed)
		finalSpeed = maxSpeed;
	if (finalSpeed < minSpeed)
		finalSpeed = minSpeed;
}

void rd::AnimBus::im() {
	using namespace ImGui;
	PushID(this);
	if (!name.empty())
		Text(name.c_str());
	DragDouble("min speed", &minSpeed,0.1,0.0f,8.0f);
	DragDouble("max speed", &maxSpeed,0.1,0.0f,8.0f);
	DragDouble("bus speed", &speed,0.1,0.0f,8.0f);
	Value("final speed", finalSpeed);
	PopID();
}

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
	return anm->plays == 0;
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
	stack = rs::Std::map<rd::TileAnim*, rd::TileAnim* > (ot.stack, [](rd::TileAnim* a) {
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
		rd::Pools::anims.release(anim);
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
		getLastAnim()->plays = -1;
	return this;
}

TileAnimPlayer * TileAnimPlayer::playAndLoop(const char * k) {
	return play(k)->loop();
}

TileAnimPlayer * TileAnimPlayer::play(const char * group, int nbPlays, bool queueAnim) {
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
	a->reset(group, g, 0, 0, nbPlays, false, 1.0);
	
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
	return a->groupData->anim[a->cursor];
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
			a->cursor = cursor;
			spr->setFrame(a->cursor);
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
}

double rd::TileAnimPlayer::getFinalAnimSpeed() {
	double libSpeed = lib ? lib->speed : 1.0;
	if (bus != nullptr)
		libSpeed *= bus->getFinalSpeed();
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
	onFrame = {};
	destroyed = true;
	if (!stack.size()) return;

	for (auto anim : stack)
		rd::Pools::anims.release(anim);

	stack.clear();
	bus = nullptr;
	if (ext)ext->reset();
}

void rd::TileAnimPlayer::reset(){
	bus = 0;
	needUpdates = true;
	destroyed = false;
	isPlaying = false;
	speed = 1.0;
	dispose();
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
		if (onFrame) onFrame(a->cursor);
	}
}

void rd::TileAnimPlayer::dumpState(){
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
	if (onFrame) {
		if(a->groupData->anim.size())
			onFrame(a->groupData->anim[0]);
		else 
			onFrame(0);
	}

	if (debug && a) {
		if (rd::String::containsI(a->groupName, "idle"))
			int here = 0;
		history.push_back(*a);
	}

}

void TileAnimPlayer::_update(double dt){
	double libSpeed  = lib ? lib->speed : 1.0;
	auto cur = getCurrentAnim();

	if (cur && !cur->paused && isPlaying ) {
		if (cur->delayMs > 0) {
			cur->delayMs-= dt * 1000.0;
			if (cur->delayMs < 0) cur->delayMs = 0;
			return;
		}

		TileAnim & a = *cur;
		double rframe = dt * frameRate;
		double fspeed = getFinalAnimSpeed();
		a.curFrameCount += fspeed * rframe;

		while (a.curFrameCount > 1) {
			a.curFrameCount--;
			a.cursor++;

			if (a.cursor >= a.nbFrames()) {
                if (a.plays > 0)
                    a.plays--;

                bool animContinue = a.plays == -1 || a.plays > 0; // continue if loop or if there is 1+ more play remaining

				if (animContinue) {
                    a.cursor = 0;
                    if (a.onLoop != nullptr) {
                        auto cb = a.onLoop;
                        a.onLoop = nullptr;
                        if (cb(this))
                            return;
                    }
				} else {
					if (ext) ext->onAnimEnd.trigger();

					if (a.onEnd != nullptr) {
						auto cb = a.onEnd;//copy is voluntary
						a.onEnd = nullptr;
						if (cb(this))
							return;
					}
					
					if (hasAnim())
						nextAnim();
					else
						break;

					continue;
				}
			}

			if (spr->getFrame() != a.cursor) {
				spr->setFrame(a.cursor);
				if (onFrame) onFrame(a.cursor);
				if (ext) ext->onAnimFrame.trigger();
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
		rd::Pools::anims.release(anim);
	stack.clear();
}

bool TileAnimPlayer::hasAnim(){
	return stack.size()>0;
}

void rd::TileAnimPlayer::nextAnim(){
	auto cur = getCurrentAnim();

	
	{
		TileAnim * anm = stack.at(0);
		anm->clear();
		rd::Pools::anims.release(anm);
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
	if (stack.size() == 0) {
		fun(this);//whoops no anim let's fulfill contract
		return;
	}
	
	getLastAnim()->onEnd = fun;
}

void rd::TileAnimPlayer::setOnLoop(std::function<bool(rd::TileAnimPlayer*)> fun) {
	if (stack.size() == 0) {
		fun(this);//whoops no anim let's loop already... but once?
		return;
	}

    getLastAnim()->onLoop = fun;
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
    ImGui::PushID("TileAnimPlayer");
	Checkbox("debug", &debug);
	Text("%d / %d", cur->cursor + 1, cur->nbFrames());
	Value("Duration", getDuration());
    if (cur->plays == -1)
        Text("Loop");
	else
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

	if(history.size())
	if (TreeNode("History")) {
		for (auto& a : history) 
			a.im();
		TreePop();
	}
	ImGui::PopID();
}

void rd::TileAnimPlayerExtension::reset()
{
	onAnimFrame.clear();
	onAnimEnd.clear();
}
