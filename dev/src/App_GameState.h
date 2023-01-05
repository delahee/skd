#pragma once

#include "2-application/GameState.h"
#include "platform.h"

class PixelScene;

namespace Pasta { class Graphic; }

class App_GameState : public Pasta::GameState{
public:

	static			App_GameState* me;

					App_GameState();
	virtual			~App_GameState();

	virtual void	load();
	virtual void	release();

	virtual int		executeGameState(Pasta::Duration deltaTime);
	virtual void	paint(Pasta::Graphic *g);

	void			onResize(const r::Vector2i& sz);
	void			testGame();
	void			testUnit();

	r2::Scene *		mainScene = nullptr;
};
