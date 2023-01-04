#include "stdafx.h"
#include "Display.hpp"
#include "2-application/Application.h"
#include "1-device/DeviceMgr.h"

int rs::Display::initialWidth()
{
	return Pasta::Application::getSingleton()->getWindowWidth();
}

int rs::Display::initialHeight()
{
	return Pasta::Application::getSingleton()->getWindowHeight();
}

int rs::Display::width()
{
	return Pasta::DeviceMgr::getSingleton()->getDevicePixelWidth();
}

int rs::Display::height()
{
	return Pasta::DeviceMgr::getSingleton()->getDevicePixelHeight();
}

