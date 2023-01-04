//! #version 140
#include "shaders/common/platform.glsl" //! #include "../common/platform.glsl"
#include "shaders/common/utils.glsl" //! #include "../common/utils.glsl"

#define POINT_LIGHT_MAX 4

vec2 poissonDisk[4] = vec2[](
  vec2( -0.94201624, -0.39906216 ),
  vec2( 0.94558609, -0.76890725 ),
  vec2( -0.094184101, -0.92938870 ),
  vec2( 0.34495938, 0.29387760 )
);

struct PointLight {
  vec3 position;

  float intensity;
  vec3 color;

  float constant;
  float linear;
  float quadratic;

  vec2 pad;
};

struct DirLight {
  vec3 position;
  float intensity;
  vec3 color;
  float pad;
};

in vec2 oTexcoord;
in vec4 oWorldPosition;
in vec4 oWorldNormal;
in vec4 oColor;

#ifndef EARLY_DEPTH
in vec4 oShadowCoord;
in vec3 oVertexLighting;
#endif

uniform sampler2D uTexture0;
uniform sampler2D uTexture1;
BEGIN_UNIFORM_BLOCK
UNIFORM vec3 uAmbientColor;
UNIFORM float uTime;

UNIFORM DirLight uDirLight;
UNIFORM PointLight uPointLights[POINT_LIGHT_MAX];
END_UNIFORM_BLOCK

out vec4 fragColor;

#ifndef EARLY_DEPTH
float computeShadow(vec3 N, vec3 L) {
	vec3 projCoords = oShadowCoord.xyz / oShadowCoord.w;
	projCoords = (projCoords * 0.5) + 0.5;

	float shadow = 0.0;

	float cosTheta = clamp(dot(N, L), 0, 1);
	float bias = 0.001 * tan(acos(cosTheta));
	bias = clamp(bias, 0.0, 0.01);
	for (int i=0;i<4;i++) {
	  if (texture2D(uTexture1, projCoords.xy + poissonDisk[i] / 1500.0).r < projCoords.z - bias) {
		shadow+=0.25;
	  }
	}
	return shadow;
}

vec3 computeDirectionalLight(vec3 N) {
	vec3 L = normalize(-uDirLight.position);

	float NdotL = clamp(sign(dot(N, L)), 0, 1);
	vec3 diffuseColor = uDirLight.color * uDirLight.intensity * NdotL;

	float shadow = computeShadow(N, L);
	return /* ambient + */ (1 - shadow) * (diffuseColor /*+ specularColor*/);
}

vec3 computeAllPointLights(vec3 N) {
	vec3 diffuse = vec3(0, 0, 0);
	for (int i = 0; i < POINT_LIGHT_MAX; i++) {
		vec3 L = normalize(uPointLights[i].position - oWorldPosition.xyz);
		float NdotL = max(dot(N, L), 0.0);
		
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

float find_closest(float x, float y, float c0) {
	int px = int(mod(x, 8.0));
	int py = int(mod(y, 8.0));

	float dither[64] = float[64](
		00.0, 32.0, 08.0, 40.0, 02.0, 34.0, 10.0, 42.0, /* 8x8 Bayer ordered dithering */
		48.0, 16.0, 56.0, 24.0, 50.0, 18.0, 58.0, 26.0, /* pattern. Each input pixel */
		12.0, 44.0, 04.0, 36.0, 14.0, 46.0, 06.0, 38.0, /* is scaled to the 0..63 range */
		60.0, 28.0, 52.0, 20.0, 62.0, 30.0, 54.0, 22.0, /* before looking in this table */
		03.0, 35.0, 11.0, 43.0, 01.0, 33.0, 09.0, 41.0, /* to determine the action. */
		51.0, 19.0, 59.0, 27.0, 49.0, 17.0, 57.0, 25.0,
		15.0, 47.0, 07.0, 39.0, 13.0, 45.0, 05.0, 37.0,
		63.0, 31.0, 55.0, 23.0, 61.0, 29.0, 53.0, 21.0
	);

	float limit = (dither[px + 8 * py] + 1.0) / 64.0;
	if(c0 < limit)
		return 0.0;
	return 1.0;
}


void main() {
	float gx =  oWorldPosition.x / 18.0 + oWorldPosition.y / 18.0 - oWorldPosition.z / 18.0; gx *= 0.5;
	float gy = -oWorldPosition.x / 18.0 + oWorldPosition.y / 18.0 - oWorldPosition.z / 18.0; gy *= 0.5;
	float gz =						    - oWorldPosition.y / 18.0 - oWorldPosition.z / 18.0;
	
	float hack = 1.0 - oColor.r;
	float fow = oColor.b;

	vec2 uv = oTexcoord;
	uv.y += (rand(oWorldPosition.x + uTime) - 0.5) * 0.004 * hack;
	
	float alpha = clamp((gz + 20) / 10.0, 0, 1);
	//float dith = find_closest(gx * 18.0 + gy * 18.0, gz * 9.0, alpha);
	float dith = find_closest(oWorldPosition.x, oWorldPosition.y, alpha);

	vec4 texColor = texture2D(uTexture0, uv);
	texColor.a *= oColor.a * dith;
	if (texColor.a <= 0.0)
		discard;
	texColor.rgb *= fow;

#ifdef EARLY_DEPTH
	fragColor = vec4(1.0, 1.0, 1.0, 1.0);
#else

	texColor.rb += (noise(gz * 2 + uTime * 10.0)) * hack;

	//texColor.r += (noise(gx * 20 + uTime * 10.0)) * hack;
	//texColor.g += (noise(gy * 15 + uTime * 8.0)) * hack;
	//texColor.b += (noise(gz * 30 + uTime * 8.0)) * hack;
	//texColor.g += (noise(gy) + 1.0) * hack * 0.1;
	//texColor.b += (noise(gz) + 1.0) * hack * 0.1;



	//vec3 N = normalize(cross(dFdx(oWorldPosition.xyz), dFdy(oWorldPosition.xyz)));
	vec3 N = normalize(oWorldNormal.xyz);
	//texColor.rgb = (N.xyz + 1.0) / 2.0;
	
	// hackverse
	/*float isWall = clamp(-sign(dFdy(oWorldPosition.z)), 0, 1);
	float hackverse = 
		clamp(round(
			(noise(gl_FragCoord.x + gl_FragCoord.y / 500.0 + uTime / 4.0) - 0.6)
			* 4.0) / 4.0, 0, 1);

	texColor.rgb += vec3(0.0, 0.8, 1.0) * isWall * 8 * hackverse;
	if (hackverse == 0 && isWall > 0) discard;
	
	// hackverse shadow
	//texColor.rgb += mix(vec3(0.0, 1.6, 2.0), vec3(0,0,0), visibility);
	
	if(isWall == 0) {
		texColor.rgb = vec3(0.03, 0.08, 0.08);

		//texColor.rgb += vec3(0.12,0.43,0.49) * 
		//	clamp(round(
		//		(noise(gl_FragCoord.y + gl_FragCoord.x / 500.0 + uTime / 4.0) - 0.6)
		//		* 4.0) / 4.0, 0, 1);
		
		float mid = 500;
		float vRotation = 3.14159 / 4.0;
		vec2 pos = gl_FragCoord.xy;
		pos.y *= 2;
		vec2 rotated = vec2(cos(vRotation) * (pos.x - mid) + sin(vRotation) * (pos.y - mid) + mid,
							cos(vRotation) * (pos.y - mid) - sin(vRotation) * (pos.x - mid) + mid);
		
		vec2 posGrid = round(rotated.xy / 16);
		float randGrid = noise_vec2(vec2(rand(posGrid.x + posGrid.y * 0.24), (sin(uTime * 0.75+ posGrid.x+ posGrid.y) + 1) / 0.5));
		randGrid = clamp(map(randGrid, 0, 1, -2.5, 1.0), 0, 1);
		texColor.rgb += vec3(0.12,0.43,0.49) * 1.5 * randGrid;
	}*/

	// forcefield light
	/*vec3 forcefieldPos = vec3(80, -20, 20);
	float dist = distance(oWorldPosition.xyz, forcefieldPos);
	float light = pow(clamp(100 - distance(oWorldPosition.xyz, forcefieldPos), 0, 100) / 50, 2);
	visibility += light;*/
	
	// forcefield edge
	//texColor.rgb += smoothstep(uRadius, uRadius + 7, dist) - step(uRadius + 7, dist);

	/*{
		//float dir = (gl_FragCoord.z / 0.004);
		float dir = -(gy / 10);
		texColor.rgb += pow(fract(dir - uTime), 2) * vec3(0, 1, 1) * 0.5;
		texColor.rgb += pow(fract(-dir + uTime - 0.01), 20) * vec3(0, 1, 1) * 0.5;
	}*/

	vec3 totalLight = uAmbientColor + computeDirectionalLight(N) + oVertexLighting; // + computeAllPointLights(N);
	float height = clamp(gz, 0, 6) * 0.05;
	vec3 lightHeight = vec3(0.17, 0.44, 0.45) * height * 1.5;

	fragColor = vec4((texColor.rgb + lightHeight) * totalLight, texColor.a);
#endif
}