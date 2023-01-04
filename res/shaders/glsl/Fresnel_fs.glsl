//! #version 140
#include "shaders/common/platform.glsl" //! #include "../common/platform.glsl"

in vec4 oWorldNormal;
in vec4 oViewDir;

out vec4 fragColor;

void main() {
    vec4 N = normalize(oWorldNormal);
    vec4 V = normalize(oViewDir);

	float fresnel = pow(1.0 - clamp(dot(N, V), 0, 1), 1.0);
    
    vec4 lightDirection = normalize(vec4(0.5,-0.4,-0.2,0));
    vec4 R = 2 * N * dot(N, lightDirection) - lightDirection;
    float spec = pow(max(0.0, dot(R, V)), 50);

    vec3 color = vec3(0, 1.0, 0.8) * fresnel;
    color += spec * vec3(0, 1.0, 0.8) * 0.5;

	fragColor = vec4(color.rgb, 1);
}