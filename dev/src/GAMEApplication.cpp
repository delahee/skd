#include "stdafx.h"
#include <chrono>

#include "GAMEApplication.h"
#include "App_GameState.h"
#include "1-graphics/ShaderProgram.h"
#include "1-graphics/Graphic.h"
#include "1-input/InputMgr.h"
#include "1-time/Profiler.h"
#ifdef PASTA_OGL
#include "1-graphics/OGLGraphicContext.h"
#endif

#ifdef PASTA_WIN
#include "1-files/WinPersistentMgr.h"
#include <filesystem>
#endif

#include "PixelScene.hpp"


using namespace Pasta;

GAMEApplication::GAMEApplication() : PLATFORMApplication(true)
{
	GraphicContextSettings settings;
	settings.m_numRWBufferSlots[ShaderStage::Vertex] = 1;
	settings.m_numRWBufferSlots[ShaderStage::Compute] = 1;

	settings.m_numTextureSlots[ShaderStage::Compute] = 1;
	GraphicContext::SetSettings(settings);
#ifdef PASTA_OGL
	OGLGraphicContext::setOpenGLVersion(4, 1); //Necessary for debugging with RenderDoc
#endif

	setDeltaTimeBounds(16, 100);

	setState(0);
}

GAMEApplication::~GAMEApplication(){
}

void GAMEApplication::winInit() {
#ifdef PASTA_WIN
	wchar_t sep = std::filesystem::path::preferred_separator;
	std::wstring p = L"";
	p += sep;
	p += L"hbc";
	p += sep;

	Pasta::WinPersistentMgr* mgr = (Pasta::WinPersistentMgr*)Pasta::WinPersistentMgr::getSingleton();
	mgr->setSaveDirName(p.c_str());
#endif
}

void GAMEApplication::init(){
	PLATFORMApplication::init();
	
#ifdef PASTA_WIN
	winInit();
#endif
	
	// Setup resolution & other window settings (depth buffer / fullscreen)
	u16 width = 640 * 2.5;
	u16 height = 360 * 2.5;

#if defined(PASTA_PS4) || defined(PASTA_NX)
	width = 1920;
	height = 1080;
#endif

	WindowFlags flags = PASTA_WINDOW_NONE;
	flags |= PASTA_WINDOW_DEPTH_BUFFER;
	flags |= PASTA_WINDOW_STENCIL_BUFFER;

	setWindowParams(width, height, flags);

	// Open the window!
	showWindow(true); 
}

void GAMEApplication::close(){
	App_GameState*& appState = App_GameState::me;
	if (appState && appState->mainScene) {
		appState->mainScene->destroy();
		appState->mainScene = 0;
	}
	App_GameState::me = 0;
	PLATFORMApplication::close();
}

GameState* GAMEApplication::createGameState(int state){
	return new App_GameState();
}

int GAMEApplication::executeState(Duration deltaTime )
{
	return PLATFORMApplication::executeState(deltaTime);
}

void GAMEApplication::onResolutionChange(int _windowID){
	PLATFORMApplication::onResolutionChange(_windowID);
	if (App_GameState::me && App_GameState::me->mainScene)
		App_GameState::me->onResize(rs::Display::getSize());
}
