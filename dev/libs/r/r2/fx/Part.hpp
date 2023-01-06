#pragma once

#include "rd/Agent.hpp"

namespace r2 { 
	namespace fx {
		class Sprite;
		class Part : public rd::Agent {
		public:
			typedef rd::Agent Super;

			r2::Sprite* spr = 0;
			float		x = 0.f;
			float		y = 0.f;

			float		alpha = 1.f;
			float		stamp = 0.f;
			float		dx = 0.f;
			float		dy = 0.f;

			float		da = 0.f;	// alpha
			float		ds = 0.f;	// scale

			float		dsx = 0.f;	// scaleX
			float		dsy = 0.f;	// scaleY

			float		scaleMul = 1.f;
			float		dr = 0.f;

			float		frictX = 1.f;
			float		frictY = 1.f;

			float		gx = 0.f;
			float		gy = 0.f;

			float		bounceMul = 0.85f;
			float		prioOffset = 0.0f;

			bool		killOnLifeOut = false;
			bool		useBounds = false;
			bool		useGround = false;
			bool		killed = false;

			r2::Bounds	bounds;
			float		groundY = 0.0f;

			float		fadeOutSpeed = 0.1f;
			float		fr = 0.f;
			float		maxAlpha = 1.f;

			float						setLife(float l);
			float						getLife() { return rLife; };
			bool						isAlive() { return rLife > 0; };

			std::function<void(Part&)>	onStart;
			std::function<void(Part&)>	onBounce;
			std::function<void(Part&)>	onUpdate;
			std::function<void(Part&)>	onKill;

			void						syncPos();
			float						setDelay(float d);

			Vector2						getPos();

		public:
										Part(r2::Sprite* sp, rd::AgentList* al = nullptr);
			virtual						~Part();

			virtual	void				dispose() override;
			virtual void				update(double dt) override;
			void						kill();
			void						im();

			static eastl::vector<Part*>	ALL;

		protected:
			float						rLife = 30.0f;
			float						maxLife = 30.0f;
			float						delay = 0.0f;

		};
	}
}