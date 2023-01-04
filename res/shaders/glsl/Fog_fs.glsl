//! #version 140
#include "shaders/common/platform.glsl" //! #include "../common/platform.glsl"
#include "shaders/common/utils.glsl" //! #include "../common/utils.glsl"
#define SIZE 25.5
#define HALF_SIZE 12.75

in vec4 oLocalPosition;
in vec2 oTexcoord;

uniform sampler2D uTexture0;
BEGIN_UNIFORM_BLOCK
UNIFORM float uTime;
UNIFORM vec4 uColor;
END_UNIFORM_BLOCK

out vec4 fragColor;

float boxDist(vec2 p, vec2 size, float radius) {
	size -= vec2(radius);
	vec2 d = abs(p) - size;
  	return min(max(d.x, d.y), 0.0) + length(max(d, 0.0)) - radius;
}

void main() {
	// vec4 trsRect1 = vec4(3, -5, 1, 1);
	// vec4 trsRect2 = vec4(8, -2.5, 5, 6);
	// vec4 trsRect3 = vec4(-2, -5, 1, 1);
	// vec4 trsRect4 = vec4(-11, -7.5, 2, 5);
	// 
    // float dist = boxDist(oLocalPosition.xy + trsRect1.xy * SIZE, trsRect1.zw * HALF_SIZE, 1);
    // dist = min(boxDist(oLocalPosition.xy + trsRect2.xy * SIZE, trsRect2.zw * HALF_SIZE, 1), dist);
    // dist = min(boxDist(oLocalPosition.xy + trsRect3.xy * SIZE, trsRect3.zw * HALF_SIZE, 1), dist);
    // dist = min(boxDist(oLocalPosition.xy + trsRect4.xy * SIZE, trsRect4.zw * HALF_SIZE, 1), dist);
	// ajouter autant de ligne que le nombre de bloc que l'on veut
	// fragColor = vec4(dist, dist, dist, 1);
	
	// float noiseMask = noise_vec2(oLocalPosition.xy / 20 + vec2(0, uTime / 4.0));
	// on clamp entre 0 et 1 avec un fade a 14 de distance des blocs:
	// float mask = clamp(dist - noiseMask * HALF_SIZE, 0, SIZE) / SIZE;
	// mask = pow(mask, 2);
	// fragColor = vec4(mask, mask, mask, 1);
	float mask = 1;

	float maskScreen = clamp(1 - gl_FragCoord.y / 900, 0, 1);
	//fragColor = vec4(maskScreen,maskScreen,maskScreen,1);
	mask = mask * maskScreen;

	vec3 posNoise1 = vec3(oLocalPosition.y / 100 + uTime / 100, oLocalPosition.x / 50 + uTime / 9, uTime / 4.0);
	vec3 posNoise2 = vec3(oLocalPosition.y / 50 + uTime / 50, oLocalPosition.x / 25 + uTime / 7, uTime / 3.0);
	//posNoise1.xy /= 2;
	posNoise1.x /= 4;
	posNoise2.xy /= 2;
	posNoise2.x /= 4;
	float noiseFog = noise_vec3(posNoise1) + noise_vec3(posNoise2);
	noiseFog *= 0.75;

	//noiseFog = 1 - clamp(noiseFog, 0, 1);
	//fragColor = vec4(noiseFog, noiseFog, noiseFog, 1);
	
	vec2 uv = oTexcoord.yx * 100 + vec2(uTime / 4.0, 0);

	vec4 fogTex = texture2D(uTexture0, uv).rgba;
	fragColor = vec4(fogTex.a * uColor.rgb * noiseFog, fogTex.a * uColor.a * mask* noiseFog);
}