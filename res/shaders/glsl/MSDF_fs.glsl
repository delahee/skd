//! #version 140
#include "shaders/common/platform.glsl" //! #include "../common/platform.glsl"

#ifdef VERTEX_COLOR
in vec4 oColor;
#endif

#if defined(TEXTURE) || defined(TEXTURE_ALPHA_ONLY)
in vec2 oTexcoord;
uniform sampler2D uTexture0;
#endif

#ifdef UNIFORM_COLOR
BEGIN_UNIFORM_BLOCK
UNIFORM vec4 uColor;
END_UNIFORM_BLOCK
#endif

out vec4 fragColor;

float median(float r, float g, float b) {
	return max(min(r, g), min(max(r, g), b));
}

void main() {
#if !defined(TEXTURE)
	vec4 color = vec4(1.0,0.0,1.0,1.0); // can't have MSDF without texture
	fragColor = color;
#else
	vec3 sample = texture2D(uTexture0, oTexcoord).rgb;
	float signedDistance = median(sample.r, sample.g, sample.b) - 0.5;
	float alpha = clamp(signedDistance/fwidth(signedDistance) + 0.5, 0.0, 1.0);

	vec4 color = vec4(1.0, 1.0, 1.0, alpha);

	#if defined(KILL_ALPHA)
		if( color.a <= 0.0)
			discard;
	#endif

	#if   !defined(VERTEX_COLOR) && !defined(UNIFORM_COLOR)
		fragColor = color;
	#elif  defined(VERTEX_COLOR) && !defined(UNIFORM_COLOR)
		fragColor = color * oColor;
	#elif !defined(VERTEX_COLOR) &&  defined(UNIFORM_COLOR)
		fragColor = color * uColor;
	#elif  defined(VERTEX_COLOR) &&  defined(UNIFORM_COLOR)
		fragColor = color * oColor * uColor;
	#endif

#endif
}