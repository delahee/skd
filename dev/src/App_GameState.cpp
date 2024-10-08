﻿#include "stdafx.h"

#include "App_GameState.h"
#include "PixelScene.hpp"
#include "GAMEApplication.h"
#include "r2/im/HierarchyExplorer.hpp"
#include "Cine.hpp"
#include "Data.hpp"
#include "Game.hpp"

using namespace rd;
App_GameState* App_GameState::me = 0;

static void updateTools();

static void bootGame(r2::Node * root) {
	auto app = App_GameState::me;
	auto sc = app->mainScene;

	new Game(root,sc,&sc->al);
	new AnonAgent( updateTools, &sc->al);

#ifdef PASTA_DEBUG
	r2::im::HierarchyExplorer::toggle(App_GameState::me->mainScene);
#endif
}

void updateTools(){
#ifdef PASTA_DEBUG
	if (rs::Input::isKeyboardKeyJustReleased(Pasta::Key::KB_F1)) {
		r2::im::HierarchyExplorer::toggle(App_GameState::me->mainScene);
	}
#endif
}


static void regularBloom(PixelScene* dts, float durMs) {
	if (durMs < 0) 2;
	if (!dts) return;

	dts->uber.bloomEnabled = true;
	dts->uber.pyramidSize = 5;
	dts->tw.create(dts, (rs::TVar)PixelScene::VBLOOM_INTENSITY, 0.14, TType::TEaseOut, durMs);
	dts->tw.create(dts, (rs::TVar)PixelScene::VBLOOM_THRESH, 0.95, TType::TEaseOut, durMs);
	dts->tw.create(dts, (rs::TVar)PixelScene::VBLOOM_KNEE, 0.60, TType::TEaseOut, durMs);

	r::Color basicBiomeBloomColor(0.3f, 0.5, 0.4f);

	r::Color c = basicBiomeBloomColor;
	dts->tw.create(dts, (rs::TVar)PixelScene::VBLOOM_CR, c.r, TType::TEaseOut, durMs);
	dts->tw.create(dts, (rs::TVar)PixelScene::VBLOOM_CG, c.g, TType::TEaseOut, durMs);
	dts->tw.create(dts, (rs::TVar)PixelScene::VBLOOM_CB, c.b, TType::TEaseOut, durMs);

	dts->uber.vignetteEnabled = true;
	dts->tw.create(dts, (rs::TVar)PixelScene::VVIGNETTE_INTENSITY, 1.07, TType::TEaseOut, durMs);
	dts->tw.create(dts, (rs::TVar)PixelScene::VVIGNETTE_SMOOTHNESS, 1.270, TType::TEaseOut, durMs);
	dts->tw.create(dts, (rs::TVar)PixelScene::VVIGNETTE_ROUNDNESS, 0.95, TType::TEaseOut, durMs);
}

void App_GameState::testUnit(){
	mainScene = new r2::Scene();
	auto t = new r2::Text(nullptr, "SAPIN!", mainScene);
	t->x = 100;
	t->y = 100;
}

void App_GameState::testGame(){
	if(mainScene == nullptr){
		PixelScene* ps = nullptr;
		ps = new PixelScene(Cst::W,Cst::H);
		ps->clearColor = r::Color::Blue;
		ps->doClear = true;
		regularBloom(ps,-1);
		mainScene = ps;
		float s = (int)rs::Display::height() / Cst::H;
		mainScene->setZoom(s, s);

		int scx = s * Cst::W;
		int scy = s * Cst::H;

		int remSX = (rs::Display::width() / s - Cst::W) * 0.5f;
		int remSY = (rs::Display::height() / s - Cst::H) * 0.5f;

		//mainScene->setPan( -remSX, -remSY);
		auto n = r2::Node::fromPool(ps);
		n->setPos(remSX, remSY);
		n->name = "map";
		auto stage = r2::Node::fromPool(ps);
		stage->name = "stage";
		bootGame(n);
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

	
	//testUnit();
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
	if (mainScene)
		mainScene->render(g);

	rs::Sys::exitFrame();
}
