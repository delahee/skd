//! #version 140
#include "shaders/common/platform.glsl" //! #include "../common/platform.glsl"

in vec4 oColor;

BEGIN_UNIFORM_BLOCK
UNIFORM vec4 uColor;
UNIFORM float uTime;
END_UNIFORM_BLOCK

out vec4 fragColor;

void main() {
	float rampSpeed = 2.0;
	float rampIntensity = 1.5;
	
	float ramp = 1.0 - fract(uTime * rampSpeed - oColor.a);
#ifdef VIEW_THROUGH
	if(mod(ramp * 8, 4) < 2) discard;
#endif
	fragColor.rgb = uColor.rgb * oColor.rgb * (1 + ramp * rampIntensity);
	fragColor.a = uColor.a * 0.5;
}