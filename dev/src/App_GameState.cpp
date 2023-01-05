#include "stdafx.h"

#include "App_GameState.h"
#include "PixelScene.hpp"
#include "GAMEApplication.h"
#include "r2/im/HierarchyExplorer.hpp"
#include "Cine.hpp"
#include "Data.hpp"
#include "Game.hpp"

App_GameState* App_GameState::me = 0;

static void updateTools();

static void bootGame() {
	auto app = App_GameState::me;
	auto sc = app->mainScene;

	new Game(sc,&sc->al);
	new AnonAgent( updateTools, &sc->al);
	r2::im::HierarchyExplorer::toggle(App_GameState::me->mainScene);
}

void updateTools(){
	if (Input::isKeyboardKeyJustReleased(Pasta::Key::KB_F1)) {
		r2::im::HierarchyExplorer::toggle(App_GameState::me->mainScene);
	}

}

void App_GameState::testGame(){
	if(mainScene == nullptr){
		//mainScene = new PixelScene(Cst::W,Cst::H);
		mainScene = new r2::Scene();
		float s = (int)rs::Display::height() / Cst::H;
		mainScene->setZoom(s, s);

		int scx = s * Cst::W;
		int scy = s * Cst::H;

		int remSX = (rs::Display::width() / s - Cst::W) * 0.5f;
		int remSY = (rs::Display::height() / s - Cst::H) * 0.5f;

		mainScene->setPan( -remSX, -remSY);
		bootGame();
	}
}

void App_GameState::onResize(const r::Vector2i& sz) {
	if (mainScene)
		mainScene->onResize(sz);
}

App_GameState::~App_GameState() {
	int here = 0;
}

App_GameState::App_GameState() {
	me = this;

	r2::Lib::init();
	ri18n::T::init();

	GAMEApplication* app = (GAMEApplication*)GAMEApplication::getSingleton();
	if (app->params.find("game") != app->params.end()) {
		return;
	}
	if (app->params.find("tool") != app->params.end()) {
		std::string tool = app->params["tool"];
		return;
	}

	if (app->params.find("test") != app->params.end()) {
		std::string levelName = app->params["test"];
		
		return;
	}

	

	testGame();
}

void App_GameState::load() {

}


int App_GameState::executeGameState(Pasta::Duration deltaTime) {
	rs::Sys::enterFrame();

	int df = rs::Timer::df;
	double dt = rs::Timer::dt;

	if (mainScene) {
		mainScene->checkEvents();
		mainScene->update(dt);
	}

	return -1;
}


void App_GameState::release() {
	
	if (mainScene) {
		mainScene->al.deleteAllChildren();
		mainScene->destroy(); mainScene = nullptr;
	}
	r2::Lib::dispose();
	rs::Sys::dispose();
}

void App_GameState::paint(Pasta::Graphic* g) {
	g->setClear(false);
#ifdef _DEBUG
	auto color = r::Color(0.1f, 0.5f, 0.1f, 1.0f);
#else
	auto color = r::Color(0.0f, 0.0f, 0.0f, 1.0f);
#endif
	auto clearAll = PASTA_CLEAR_COLOR | PASTA_CLEAR_DEPTH | PASTA_CLEAR_STENCIL;
	Pasta::GraphicContext::GetCurrent()->clear(clearAll, color, GfxContext::MTX_DEFAULT_ZMIN());

	if (mainScene)
		mainScene->render(g);

	rs::Sys::exitFrame();
}
