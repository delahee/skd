#include "stdafx.h"

#include "JSerialize.hpp"
#include "AudioEmitter2D.hpp"
#include "AudioMan.hpp"

using namespace std;
using namespace rd;

#define SUPER r2::Node

AudioEmitter2D::AudioEmitter2D(r2::Node*p) : SUPER(p){
	name = string() + "audio emitter#" + to_string(uid);

	auto& am = AudioMan::get();
	forward = -am.listenerFwd;
	up = am.listenerUp;
}

AudioEmitter2D::~AudioEmitter2D() {
	if (eventInstance) {
		eventInstance->release();
		eventInstance = 0;
	}
}

void AudioEmitter2D::debugDraw() {
	float size = 15;
	if( isPlaying())
		size = 15 + pow(fmod(rs::Timer::getTimeStamp(),1.0),0.5) * 10;
	auto g = r2::Im::circle(Vector2(0, 0), size, this, 3, 5);
	g->name = string("audio slave#") + to_string(uid);
}

void AudioEmitter2D::sync3dAttr() {
	if (eventInstance) {
		FMOD_3D_ATTRIBUTES attrs = {};

		forward.normalize();
		up.normalize();

		attrs.forward = { forward.x,forward.y,forward.z };
		attrs.up = { up.x,up.y,up.z };
		attrs.position = { (float)x, (float)y , (float)z };
		attrs.velocity = { velocity.x,velocity.y,velocity.z };
		FMOD_CHECK(eventInstance->set3DAttributes(&attrs));
	}
}

void rd::AudioEmitter2D::update(double dt){
	if (showEmitterDebugDraw)
		debugDraw();
	SUPER::update(dt);
}

void AudioEmitter2D::im() {
	using namespace ImGui;
	if (CollapsingHeader("Emitter Properties", 0)) {
		Indent();
		imEmitter();
		Unindent();
	}
	SUPER::im();
}


void AudioEmitter2D::updateVolume() {
	if (eventInstance)
		eventInstance->setVolume(volume);
}

void AudioEmitter2D::updatePitch() {
	if (eventInstance)
		eventInstance->setPitch(pitch);
}

void AudioEmitter2D::onEventParameterChanged(pair<FMOD_STUDIO_PARAMETER_DESCRIPTION, float>& prm){
	FMOD_CHECK(eventInstance->setParameterByID(prm.first.id, prm.second));
}

void AudioEmitter2D::serialize(Pasta::JReflect& jr, const char* name) {
	SUPER::serialize(jr, name);
	if (name) jr.visitObjectBegin(name);

	jr.visit(event, "event");
	jr.visit(bank, "bank");
	if (rd::String::contains(bank, "\\"))
		bank = rd::String::replace(bank, "\\", "/");

	jr.visit(directionYaw, "directionYaw");
	jr.visit(volume, "volume");
	jr.visit(pitch, "pitch");
	jr.visit(up, "up");
	//jr.visit(useEmitterReverbs, "useEmitterReverbs");
	//jr.visit(reverbs, "reverbs");

	bool serIsPlaying = isPlaying();
	jr.visit(serIsPlaying, "isPlaying");
	if (jr.isReadMode()) {

		auto& sm = AudioMan::get();
		if (serIsPlaying) {
			//restore playing ?
			//may be add control from above
		}
	}

	if (jr.m_read)
		sync3dAttr();
}

void AudioEmitter2D::onEventChange() {
	AudioMan& sm = AudioMan::get();
	auto& fmodSys = sm.fmodSystem;
	FMOD_RESULT res = FMOD_OK;
	sm.getEvent(event.c_str(), eventDesc);
	if (eventDesc) {
		params.clear();
		int nbPrm = 0;
		res = eventDesc->getParameterDescriptionCount(&nbPrm); FMOD_CHECK(res);
		params.resize(nbPrm);
		if (nbPrm) {
			for (int i = 0; i < nbPrm; ++i) {
				FMOD_STUDIO_PARAMETER_DESCRIPTION tmp = {};
				res = eventDesc->getParameterDescriptionByIndex(i, &tmp);
				FMOD_CHECK(res);
				params[i].first = tmp;
				params[i].second = tmp.defaultvalue;
			}
		}
	}
}

r2::Bounds rd::AudioEmitter2D::getMyLocalBounds()
{
	r2::Bounds b;
	b.empty();
	syncAllMatrix();

	Pasta::Matrix44 local = getLocalMatrix();

	float sc = 20;
	Pasta::Vector3 topLeft = local * sc * Pasta::Vector3(-sc,-sc,0);
	Pasta::Vector3 topRight = local * sc * Pasta::Vector3(sc, sc , 0.0);
	Pasta::Vector3 bottomLeft = local * sc * Pasta::Vector3(-sc ,sc , 0.0);
	Pasta::Vector3 bottomRight = local * sc * Pasta::Vector3(sc, sc, 0.0);

	b.addPoint(topLeft.x, topLeft.y);
	b.addPoint(topRight.x, topRight.y);
	b.addPoint(bottomLeft.x, bottomLeft.y);
	b.addPoint(bottomRight.x, bottomRight.y);

	return b;
}

void rd::AudioEmitter2D::set(const string& ev, const string& bnk){
	event = ev;
	bank = bnk;
	onEventChange();
}

bool AudioEmitter2D::isPlaying() {
	if (!eventInstance)
		return false;

	if (eventInstance->isValid()) {
		FMOD_STUDIO_PLAYBACK_STATE st = FMOD_STUDIO_PLAYBACK_STOPPED;
		if (FMOD_OK == eventInstance->getPlaybackState(&st)) {
			if (st != FMOD_STUDIO_PLAYBACK_STOPPED)
				return true;
		}
	}
	if (eventInstance)
		eventInstance->release();
	eventInstance = 0;
	return false;
}

void AudioEmitter2D::stop() {
	eventInstance->release();
	eventInstance->stop(FMOD_STUDIO_STOP_ALLOWFADEOUT);
	eventInstance = nullptr;
}


void AudioEmitter2D::play() {
	if (isPlaying())
		stop();
	AudioMan::get().getEventInstance(event.c_str(), eventInstance);
	if (eventInstance) {
		eventInstance->start();
		sync3dAttr();
		updateVolume();
		updatePitch();
		error.clear();
	}
	else {
		error = "Cannot retrieve instance";
	}
}


void AudioEmitter2D::imEmitter() {
	using namespace ImGui;

	bool update3dAttr = false;
	auto& sm = AudioMan::get();

	{
		auto bnk = sm.getBank(bank.c_str());
		if (!bnk)
			error = "No such _bank_, ensure it is loaded with same path";
		else {
			FMOD::Studio::EventDescription* ed = sm.getEvent(event.c_str());
			if (!ed)
				error = "No such _event_, ensure it is loaded with same path";
			else
				error = "";
		}
	}

	if (DragFloat("Volume", &volume, 0.01f, 0.0f, 1.0f))
		updateVolume();
	if (DragFloat("Pitch", &pitch, 0.01f, 0.0f, 10.0f))
		updatePitch();

	if (DragFloat3("velocity", velocity.ptr(), 0.125f, -100.0f, 100.0f))
		update3dAttr = true;


	bool pwrChanged = 0;
	pwrChanged |= SliderAngle("Direction", &directionYaw, -360, 360);;
	if (pwrChanged) {
		Vector3 front(0, 1, 0);
		front = front.getAroundZAxisRotated(PASTA_RAD2DEG(directionYaw));
		forward = front;
		update3dAttr = true;
	}

	if (DragFloat3Col(r::Color(0,1,0), "up", up.ptr(), 0.125f, -100.0f, 100.0f)) {
		update3dAttr = true;
	}

	if (eventDesc) {
		Separator();
		Text("Event is ready");
		float minDist = 0.0f, maxDist = 0.0f;
		bool is3D = false;
		if (FMOD_OK == eventDesc->getMinMaxDistance(&minDist, &maxDist)) {
			LabelText("Min Dist", "%f", minDist);
			LabelText("Max Dist", "%f", maxDist);
		}
		if (FMOD_OK == eventDesc->is3D(&is3D))
			Text(is3D ? "3D" : "2D");
		else
			Text("Cannot retrieve 3d info");
	}

	if (eventInstance && isPlaying()) {
		Separator();
		Text("Playing...");
	}

	if (update3dAttr)
		sync3dAttr();

	Vector3			velocity;
	Vector3			forward;
	Vector3			up;

	LabelText("Audio Event", "%s", event.c_str());
	if (bank.length())
		LabelText("Bank", "%s", bank.c_str());
	if (Button("Pick Event")) {
		ImGui::OpenPopup("Pick audio event");
		eventPickerOpened = true;
	}

	if (eventPickerOpened) {
		ImGui::SetNextWindowSize(ImVec2(500, 440), ImGuiCond_FirstUseEver);
		if (BeginPopupModal("Pick audio event", &eventPickerOpened)) {
			AudioMan& sm = AudioMan::get();
			if (!sm.banks.size())
				Text("No banks loaded, open Audio Explorer ( F3 )");

			for (auto& bnk : sm.banks) {
				if (strstr(bnk.first.c_str(), ".strings")) continue;

				if (CollapsingHeader(bnk.first.c_str())) {
					auto events = sm.getEventList(bnk.first.c_str());
					for (int ev = 0; ev < events.size(); ++ev) {
						if (Selectable(events[ev].c_str(), false)) {
							bank = bnk.first;
							event = events[ev];

							onEventChange();
						}
					}
				}
			}
			EndPopup();
		}
	}

	Separator();
	if (Button(ICON_MD_PLAY_ARROW)) play(); SameLine();
	if (Button(ICON_MD_STOP)) stop(); SameLine();
	NewLine();
	Separator();
	Text("FMOD Parameters");
	for (auto& prm : params) {
		//todo show other attrs
		float delta = prm.first.maximum - prm.first.minimum;
		if (DragFloat(prm.first.name, &prm.second, delta / 100.0f, prm.first.minimum, prm.first.maximum)) {
			if (eventInstance) {
				onEventParameterChanged(prm);
			}
		}
	}

	Separator();

	debugDraw();

}

#undef SUPER
