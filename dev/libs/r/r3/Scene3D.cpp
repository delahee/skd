#include "stdafx.h"

#include "Scene3D.hpp"

r3::Scene3D::Scene3D() : r2::Scene() {
	persW = rs::Display::width();
	persH = rs::Display::height();
}

void r3::Scene3D::stdMatrix(rs::GfxContext* ctx, int w, int h) {
	if (ctx) {
		ctx->zMin = zMin;
		ctx->zMax = zMax;
	}

	if (w == -1) w = windowWidth();
	if (h == -1) h = windowHeight();

	persW = w * cameraScale.x;
	persH = h * cameraScale.y;

	viewMatrix = Matrix44::cameraLookAt(cameraPos, cameraLookAt, worldUp);
	projMatrix = Matrix44::frustum(angleDeg, w, h, ctx->zMin, ctx->zMax);

	if (ctx) {
		ctx->loadViewMatrix(viewMatrix);
		ctx->loadModelMatrix(Matrix44::identity);
		ctx->loadProjMatrix(projMatrix);

		Pasta::GraphicContext* gfxCtx = ctx->getGpuContext();
		s16 x; s16 y; u16 width; u16 height;
		gfxCtx->getViewport(&x, &y, &width, &height);
		width *= cameraScale.x;
		height *= cameraScale.y;
		ctx->setViewport(
			offset.x - (offsetFromOrigin ? width / 2 : 0),
#ifdef PASTA_OGL
			-offset.y - (offsetFromOrigin ? height / 2 : 0),
#else
			 offset.y,
#endif
			width, height
		);
	}
}

void r3::Scene3D::update(double dt) {
	viewMatrix = Matrix44::cameraLookAt(cameraPos, cameraLookAt, Vector3(0, -1, 0));
	//viewMatrix.setScale(cameraScale);

	al.update(dt);
	for (UpdateOp& ro : preUpdateOps) ro(dt);
	Node::update(dt);
	for (UpdateOp& ro : postUpdateOps) ro(dt);
}

InputEvent r3::Scene3D::transformEvent(InputEvent& ev) {
	float imageAspectRatio = persW / persH;

	Pasta::GraphicContext* gfxCtx = Pasta::GraphicContext::GetCurrent();
	s16 x; s16 y; u16 width; u16 height;
	gfxCtx->getViewport(&x, &y, &width, &height);
	width *= cameraScale.x;
	height *= cameraScale.y;
	float screenX = ev.relX - offset.x + (offsetFromOrigin ? width / 2 : 0);
	float screenY = ev.relY + 
#ifdef PASTA_OGL
		 offset.y + (offsetFromOrigin ? height / 2 : 0);
#else
		-offset.y - (offsetFromOrigin ? height / 2 : 0);
#endif
		

	// Px & Py are the ray direction in camera space
	float Px = (2 * (screenX / persW) - 1) * tanf(PASTA_DEG2RAD(angleDeg / 2)) * imageAspectRatio;
	float Py = (1 - 2 * (screenY / persH)) * tanf(PASTA_DEG2RAD(angleDeg / 2));

	// Switch to world space with the inversed view matrix
	Pasta::Vector3 pos = viewMatrix.inverse() * Pasta::Vector3::Zero;
	Pasta::Vector3 dir = viewMatrix.inverse() * Pasta::Vector3(Px, Py, -1) - pos;
	dir.normalize();

	// Give the distance from the camera to a plane at the origin facing the Z axis
	float dist = (-pos * Pasta::Vector3::AxisZ) / (Pasta::Vector3::AxisZ * dir);
	Pasta::Vector3 intersec = pos + dir * dist;

	InputEvent evSub(ev);
	evSub.relX = intersec.x;
	evSub.relY = intersec.y;
	return evSub;
}