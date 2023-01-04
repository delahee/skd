#pragma once

#include "r/Types.hpp"
#include "r2/Node.hpp"

#include "fmod_studio.hpp"

namespace rd {
	class AudioEmitter2D : public r2::Node {
	public:
							AudioEmitter2D(r2::Node*p=0);
		virtual				~AudioEmitter2D();

		FMOD::Studio::EventDescription*	eventDesc = 0;
		FMOD::Studio::EventInstance*	eventInstance = 0;
		bool							eventPickerOpened = false;
		std::string						event;
		std::string						bank;
		std::string						error;
		float							pitch = 1.0f;
		float							volume = 1.0f;
		float							directionYaw = 0.0f;
		r::Vector3						velocity;
		r::Vector3						forward = r::Vector3(0, 1, 0);
		r::Vector3						up = r::Vector3(0, 0, 1);
		bool							showEmitterDebugDraw = true;
		std::vector<std::pair<FMOD_STUDIO_PARAMETER_DESCRIPTION, float>>
			params;

		virtual r2::Bounds	getMyLocalBounds();
		void				set(const std::string& ev, const std::string& bnk);
		bool				isPlaying();
		void				play();
		void				stop();
		virtual void		debugDraw();
		
		virtual void		update(double dt) override;
		virtual void		im()override;
		void				imEmitter();

		virtual void		serialize(Pasta::JReflect& jr, const char* name = nullptr);
		void				onEventChange();
		void				updateVolume();
		void				updatePitch();
		void				onEventParameterChanged(std::pair<FMOD_STUDIO_PARAMETER_DESCRIPTION, float>& prm);
		void				setParameter(const char* name, float val);
		void				sync3dAttr();
	};
}