//! #version 140
#include "shaders/common/platform.glsl" //! #include "../common/platform.glsl"

#define POINT_LIGHT_MAX 4

struct PointLight {
  vec3 position;

  float intensity;
  vec3 color;

  float constant;
  float linear;
  float quadratic;

  vec2 pad;
};

in vec4 aPosition;
in vec2 aTexcoord0;
in vec4 aColor;
in vec4 aNormal;

BEGIN_UNIFORM_BLOCK
UNIFORM mat4 uModel;
UNIFORM mat4 uModelViewProjection;
UNIFORM mat4 uTextureMatrix;
UNIFORM mat4 uLightMVP;
UNIFORM PointLight uPointLights[POINT_LIGHT_MAX];
UNIFORM float uTime;
END_UNIFORM_BLOCK

out vec2 oTexcoord;
out vec4 oWorldPosition;
out vec4 oWorldNormal;
out vec4 oColor;

#ifndef EARLY_DEPTH
out vec4 oShadowCoord;
out vec3 oVertexLighting;

float rand(float n){return fract(sin(n) * 43758.5453123);}
float noise (float p){
	float fl = floor(p);
	float fc = fract(p);
	return mix(rand(fl), rand(fl + 1.0), fc);
}

vec3 computeAllPointLights() {
	vec3 diffuse = vec3(0, 0, 0);
	for (int i = 0; i < POINT_LIGHT_MAX; i++) {
		vec3 L = normalize(uPointLights[i].position - oWorldPosition.xyz);
		float NdotL = max(dot(oWorldNormal.xyz, L), 0.0);
		
		if (NdotL > 0.0 && uPointLights[i].intensity > 0.0) {
			float d = length(uPointLights[i].position - oWorldPosition.xyz);
			float attenuation = 1.0 / (
				uPointLights[i].constant + 
				uPointLights[i].linear * d + 
				uPointLights[i].quadratic * d * d
			);
			diffuse += uPointLights[i].color * uPointLights[i].intensity * attenuation * NdotL;
		}
	}

	return diffuse;
}
#endif

void main() {
	oTexcoord = (uTextureMatrix * vec4(aTexcoord0, 0.0, 1.0)).xy;
	oWorldPosition = uModel * vec4(aPosition.xyz, 1.0);
	oWorldNormal = uModel * aNormal;
	oColor = aColor;
	
#ifndef EARLY_DEPTH
	oShadowCoord = uLightMVP * oWorldPosition;
	oVertexLighting = computeAllPointLights();
#endif

    gl_Position = uModelViewProjection * aPosition;
	
	/*oTexcoord.x += ((noise(oTexcoord.y + oWorldPosition.z + uTime / 2.0) - 0.5) / 50.0);
	oTexcoord.y += ((noise(oTexcoord.x + oWorldPosition.y + uTime / 2.0) - 0.5) / 50.0);*/
	
	/*vec4 pos = aPosition;
	pos.x += (noise(oTexcoord.y + oWorldPosition.z + uTime / 5.0) - 0.5) * 10;
	pos.y += (noise(pos.x + oWorldPosition.y + uTime / 5.0) - 0.5) * 10;
	pos.z += (noise(oTexcoord.y + oWorldPosition.x + uTime / 5.0) - 0.5) * 10;*/
	
	/*float gz = (-wPos.y - wPos.z) / (18.0 * (0.5 + 0.17));
	float gx = wPos.x / 18.0 + wPos.y / 9.0 + gz; gx *= 0.5;
	float gy = wPos.y / 9.0 - wPos.x / 18.0 + gz; gy *= 0.5;

	gx = floor(gx);
	gy = floor(gy);
	gz = floor(gz);

	float diag = (gx + gy);
	float timing = 1 - min(mod(uTime + gl_VertexID / 1000.0, 5), 1);

	pos.y += (diag * timing * 20);

    gl_Position = uModelViewProjection * pos;*/
}