#pragma once

#include "r2/Lib.hpp"

namespace r2 {
	class HotReloadShader : public rd::Agent {
	public:
        Pasta::ShaderProgram*   sh = 0;
        r2::Shader	            shader;
        unsigned int            flags = 0;

		u64		                lastLoaded = 0;
		double		            timer = 0.0;

        HotReloadShader(Pasta::ShaderProgram* prog, rd::AgentList* al = nullptr) : rd::Agent(al), shader(r2::Shader::SH_END), sh(prog) {}

        HotReloadShader(r2::Shader shaderType, unsigned int shaderFlags = 0, rd::AgentList* al = nullptr) : rd::Agent(al), shader(shaderType), flags(shaderFlags) {
            sh = Lib::getShader(shader, flags);
		}

        virtual void update(double dt) override;
	};
}
