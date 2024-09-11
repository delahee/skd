#include "stdafx.h"
#include "Device.hpp"

using namespace rplatform;

Device* Device::me = 0;

void Device::initPlatform(){
	if (!me)//no double inits
		return;
	me = new Device();
}
