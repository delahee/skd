#pragma once

#include "r2/Scene.hpp"

namespace r3 {
	class Scene3D : public r2::Scene {
	public:
		r::Vector3	cameraLookAt;
		r::Vector3	worldUp = r::Vector3(0, -1, 0);

		// when using offsetFromOrigin the 0, 0 of the scene is placed in the upper left corner
		// the offset is in screen space in order to easily place the scene as an overlay
		bool		offsetFromOrigin = false;
		r::Vector3	offset = r::Vector3(0, 0, 0);
	public:
		float		persW = 0.0f;
		float		persH = 0.0f;
		float		zMin = 0.01f;
		float		zMax = 5000.0f;
		float		angleDeg = 120.0f;

		// does not rely on actual matrix hehe
		// the setup will position persW and persh half at center
		// which means
		// if your object is in screen space, it should stay in its place
		// if you change persw and persh you can get the perspective to be centered on your object

		Scene3D();
		virtual void			update(double dt) override;
		virtual void			stdMatrix(rs::GfxContext* ctx, int w = -1, int h = -1) override;

		virtual rs::InputEvent	transformEvent(rs::InputEvent& ev) override;

		virtual void			im() override;
	};
}