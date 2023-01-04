#include "stdafx.h"

#include "App_GameState.h"
#include "PixelScene.hpp"
#include "GAMEApplication.h"
#include "r2/im/HierarchyExplorer.hpp"
#include "Cine.hpp"

App_GameState* App_GameState::me = 0;

static void testCine() {
	auto app = App_GameState::me;
	auto sc = app->mainScene;
	auto cc = new CineController(&sc->al);

	cc->add([]() {
		trace("a");
	});

	cc->add([]() {
		trace("b");
	});

	class CustomCineStep : public CineStep{
		virtual void update(double dt) override {
			trace("c");
			finished = true;
		};
	};	
	cc->add( new CustomCineStep());
	cc->add([]() {
		trace("d");
	});
	//cc->steps.add(new WaitForMessage(""));//cc->waitForMessage();
	//cc->add(new WaitForNamedTypewriter("Cine1_step2",2.0f));//cc->waitForSeconds();
	//cc->add(new WaitForAnyInput());//cc->waitForAnyInput();
	//cc->add(new WaitForVariable("SPOT_REACHED"));//cc->waitForVariable();
	cc->add(new WaitForSeconds(5.0f));//cc->waitForSeconds();
	cc->add([]() {
		trace("e");
		trace("press space to continue");
	});
	cc->add(new WaitForKeyInput(Pasta::Key::KB_SPACE));//cc->waitForInput();
	cc->add([]() {
		trace("f");
	});
	cc->add([=]() {
		rs::Timer::delay(1000,[=]() {
			cc->msg("nope");
		});
	});
	cc->add([=]() {
		rs::Timer::delay(3000, [=]() {
			cc->msg("My message !");
		});
	});
	cc->add(new WaitForMessage("My message !"));//cc->waitForMessage();
	cc->add([]() {
		trace("g");
	});
	cc->start();
}

static void bootGame() {
	auto app = App_GameState::me;
	auto sc = app->mainScene;

	auto t = ri18n::RichText::mk("This is a *funny* text", sc);
	t->x = 300;
	t->y = 300;
	t->setScale(3, 3);

	testCine();
}


static void updateGame(double dt){
	auto app = App_GameState::me;
	auto sc = app->mainScene;

	if( Input::isKeyboardKeyJustReleased(Pasta::Key::KB_F1 )){
		r2::im::HierarchyExplorer::toggle(App_GameState::me->mainScene);
	}
	
}

void App_GameState::testGame(){
	if(mainScene == nullptr){
		mainScene = new PixelScene();
		bootGame();
	}
	new AnonAgent([](double dt) { updateGame(dt); }, &mainScene->al);
}

void App_GameState::onResize(const r::Vector2i& sz) {
	if (mainScene)
		mainScene->onResize(sz);
}


App_GameState::~App_GameState() {
	
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
	if (mainScene) mainScene->destroy(); mainScene = nullptr;
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
