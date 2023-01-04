//! #version 140
#include "shaders/common/platform.glsl" //! #include "../common/platform.glsl"
#include "shaders/common/utils.glsl" //! #include "../common/utils.glsl"

in vec2 oTexcoord;

BEGIN_UNIFORM_BLOCK
UNIFORM float uTime;
UNIFORM vec4 uColor;
END_UNIFORM_BLOCK

out vec4 fragColor;

void main() {
	float line = round(oTexcoord.y * 6.0) / 6.0;
	float ramp = fract(oTexcoord.x / 5 - uTime + noise(line * 32.5812));
	ramp = pow(ramp * 4.0, 2);

	float mask = clamp(oTexcoord.x * 10.0, 0, 1);
	mask = min(mask, clamp(10.0 - oTexcoord.x * 10.0, 0, 1));

    vec3 color = vec3(oTexcoord.x, oTexcoord.y, 0.8);

	fragColor = vec4(ramp, ramp, ramp, mask) * uColor;
}