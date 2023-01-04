//! #version 140
#include "shaders/common/platform.glsl" //! #include "../common/platform.glsl"
#include "shaders/common/utils.glsl" //! #include "../common/utils.glsl"

in vec2 oTexcoord;

BEGIN_UNIFORM_BLOCK
UNIFORM vec4 uColor;
UNIFORM float uTime;
UNIFORM float uScaleTop;
UNIFORM float uScaleBottom;

UNIFORM float uRayDensity1;
UNIFORM float uRayDensity2;
UNIFORM float uRayIntensity1;
UNIFORM float uRayIntensity2;
UNIFORM float uSpeed;
UNIFORM float uFalloff;
UNIFORM float uCutoff;
UNIFORM float uEdgeFade;
END_UNIFORM_BLOCK

out vec4 fragColor;

void main() {
	float seed = 5;

    vec2 uv = oTexcoord;
    uv.x /= map(uv.y, 0, 1, uScaleTop, uScaleBottom);
    uv.x += 0.5;

	vec2 ray1 = vec2(uv.x * uRayDensity1 * 2 + sin(uTime * 0.1 * uSpeed) * (uRayDensity1 * 0.4) + seed, 1.0);
	vec2 ray2 = vec2(uv.x * uRayDensity2 * 2 + sin(uTime * 0.2 * uSpeed) * (uRayDensity2 * 0.4) + seed, 1.0);
	
	float cut = step(uCutoff, uv.x) * step(uCutoff, 1.0 - uv.x);
	ray1 *= cut; ray2 *= cut;

	float rays = (noise_vec2(ray1) * uRayIntensity1) + (noise_vec2(ray2) * uRayIntensity2);
    
	rays *= smoothstep(0.0, uFalloff, (1.0 - uv.y)); // Bottom
	rays *= smoothstep(0.0 + uCutoff, uEdgeFade + uCutoff, uv.x); // Left
	rays *= smoothstep(0.0 + uCutoff, uEdgeFade + uCutoff, 1.0 - uv.x); // Right
    
	vec3 shine = vec3(rays) * uColor.rgb;
    fragColor = vec4(shine, rays * uColor.a);
    fragColor.rgb *= fragColor.a;
}