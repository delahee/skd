//! #version 140
#include "shaders/common/platform.glsl" //! #include "../common/platform.glsl"
#include "shaders/common/utils.glsl" //! #include "../common/utils.glsl"

in vec4 oLocalPos;
in vec4 oWorldNormal;
in vec4 oViewDir;

BEGIN_UNIFORM_BLOCK
UNIFORM vec4 uColor;
UNIFORM float uT;
END_UNIFORM_BLOCK

out vec4 fragColor;

void main() {
    vec4 N = normalize(oWorldNormal);
    vec4 V = normalize(oViewDir);
	vec4 P = normalize(oLocalPos);

	float fresnel = pow(1.0 - clamp(dot(N, V), 0, 1), 1.0);
    
    vec4 lightDirection = normalize(vec4(0.5,-0.4,-0.2,0));
    vec4 R = 2 * N * dot(N, lightDirection) - lightDirection;
    float spec = pow(max(0.0, dot(R, V)), 50);

	vec3 baseCol = uColor.rgb;
	// noise color
	/*baseCol.r += mix(baseCol.r, uColor.g, abs(noise(N.y * 7.5 + uT * 50.0)) / 2.0);
	baseCol.g += mix(baseCol.g, uColor.b, abs(noise(N.y * 4.2 + uT * 50.0 + 0.5))/ 2.0);
	baseCol.b += mix(baseCol.b, uColor.r, abs(noise(N.y * 9.1 + uT * 50.0 + 0.33)/ 2.0));

	baseCol.r = max(baseCol.r, 0);
	baseCol.g = max(baseCol.g, 0);
	baseCol.b = max(baseCol.b, 0);*/

    vec3 color = baseCol * fresnel;
    color += spec * baseCol * 0.5;

	//fragColor = vec4(color.rgb, 1);



	vec3 PBullet = vec3(0, mod(uT * 4.0, 5) - 3.0, 0);
	//vec3 PBullet = vec3(0, 0, 0);
	float line = 1.0 - clamp(distance(P.xyz, PBullet), 0, 1);
	//line = smoothstep(0.1, 0.15, line);
	float rimLine = smoothstep(0.05, 0.1, line) * (1.0 - smoothstep(0.1, 0.15, line));

	float test = smoothstep(0.05, 0.1, mod(P.y - uT, 0.2)) * (1.0 - smoothstep(0.1, 0.15, mod(P.y - uT, 0.2)));
	//test += smoothstep(0.05, 0.1, mod(P.z, 0.2)) * (1.0 - smoothstep(0.1, 0.15, mod(P.z + uT, 0.2)));
	
	float alpha = max(color.r, max(color.g, color.b))* (line + rimLine * 4 + test * line);
	fragColor = vec4(color.rgb * (line + rimLine * 4 + test * line), alpha * uColor.a);
	//fragColor = vec4(test, test, test, 1);

	
	/*float line = pow(fract(-P.y * 2 - uT) * 3, 2.0);
	line += (clamp(fract(-P.y * 2 - uT), 0.9, 0.92) - 0.9) * 1000.0;

	float d = - P.z - 0.3;
	d = clamp(d, 0, 1);
	d = pow(d, 2.0);
	
	fragColor = vec4(uColor.rgb * line, d * uColor.a);*/


	// aura
	/*float atan2 = atan(P.x, P.z) * 5.0;
	float alphaAura = P.y + 0.2 + (sin(atan2 + uT * 25.0) * 0.05);
	alphaAura = clamp(alphaAura, 0, 1);
	fragColor = vec4(uColor.rgb, alphaAura);*/
}