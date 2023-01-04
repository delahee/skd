//! #version 140
#include "shaders/common/platform.glsl" //! #include "../common/platform.glsl"

in vec4 oColor;
in vec2 oTexcoord;
in vec4 oWorldPosition;

uniform sampler2D uTexture0;
BEGIN_UNIFORM_BLOCK
UNIFORM vec4 uColor;
UNIFORM float uTime;
END_UNIFORM_BLOCK

out vec4 fragColor;

float manhattanDistance(vec2 p1, vec2 p2) {
	float d1 = abs(p1.x - p2.x);
	float d2 = abs(p1.y - p2.y);
	return d1 + d2;
}

float sdfBox(vec2 p, vec2 b) {
    vec2 d = abs(p) - b;
    return length(max(d,0.0)) + min(max(d.x,d.y),0.0);
}

void main() {
	float gz = (				  - oWorldPosition.y - oWorldPosition.z) / 18.0;
	float gx = ( oWorldPosition.x + oWorldPosition.y - oWorldPosition.z) / 18.0; gx *= 0.5;
	float gy = (-oWorldPosition.x + oWorldPosition.y - oWorldPosition.z) / 18.0; gy *= 0.5;

	vec4 color = texture2D(uTexture0, oTexcoord);
	//float ramp = mod(gx - gy - uTime, 1.0);
	//float ramp = mod(manhattanDistance(round(vec2(gx, gy)), vec2(gx, gy)) - fract(uTime), 1);
	
	float dX = abs(0.55 - fract(gx + 0.5));
	float dY = abs(0.55 - fract(gy + 0.5));
	float ramp = mod(max(dX, dY) - min(mod(uTime / 2, 3), 1) + 0.5, 1);
	//float ramp = mod(sdfBox(vec2(0.1, 0.1), fract(vec2(gx, gy)) + uTime), 1);
	/*float ramp = mod(manhattanDistance(vec2(2, -3), vec2(gx, 0)) - uTime * 2.0, 1);
	ramp += mod(manhattanDistance(vec2(2, -3), vec2(0, gy)) - uTime * 2.0, 1);
	ramp = 0.9 + ramp * 0.05;*/
	//float ramp = 0.75;
#ifdef VIEW_THROUGH
	if(mod(gl_FragCoord.x + gl_FragCoord.y - uTime * 4.0, 4) < 2) discard;
#endif
	if (oColor.a > 1.0) ramp = 0.5 + (sin(uTime * 4.0) + 1.0) * 0.25;

	//ramp = 0.5 + step(ramp, 0.5) * 0.5;
	fragColor.rgb = uColor.rgb * color.rgb * ramp;
	if (color.a <= 0.0)
		discard;
	fragColor.a = uColor.a * oColor.a;
}