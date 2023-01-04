//! #version 140
#include "shaders/common/platform.glsl" //! #include "../common/platform.glsl"

in vec4 oLocalPos;

BEGIN_UNIFORM_BLOCK
UNIFORM float uTime;
END_UNIFORM_BLOCK

out vec4 fragColor;

void main() {
	vec3 color = mix(vec3(0.5, 0.43, 0.98), vec3(0.22, 0.07, 0.34), clamp(oLocalPos.y + 2.0, 0.0, 1.0));

	fragColor = vec4(color.rgb, 1.0) ;
}