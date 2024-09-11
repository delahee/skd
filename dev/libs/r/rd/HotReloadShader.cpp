#include "stdafx.h"
#include "HotReloadShader.hpp"
using namespace r2;
using namespace Pasta;

void HotReloadShader::update(double dt) {
	timer += dt;
	if (timer > 0.1) {
        u64 lastChange = 0;
        for (int s = 0; s < ShaderStage::Count_All; s++) {
            auto shader = sh->getShader((ShaderStage::Enum)s);
            if (shader) {
                std::string path = FileMgr::getSingleton()->convertResourcePath(shader->getDesc().getPath());
                u64 time = FileMgr::getSingleton()->getLastWriteTime(path.c_str());
                lastChange = std::max<u64>(time, lastChange);
            }
        }
        
		if (lastLoaded < lastChange) {
            bool reload = ShaderProgH::getResourceMgr()->hotReload(sh->getResourceId());
            PASTA_ASSERT_MSG(reload, "cannot reload the shader");
            lastLoaded = lastChange;
		}
		timer = 0.0;
    }
}