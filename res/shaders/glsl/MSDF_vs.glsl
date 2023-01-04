//! #version 140
#include "shaders/common/platform.glsl" //! #include "../common/platform.glsl"

in vec4 aPosition;
in vec2 aTexcoord0;

BEGIN_UNIFORM_BLOCK
UNIFORM mat4 uModelViewProjection;
UNIFORM mat4 uTextureMatrix;
END_UNIFORM_BLOCK

#if defined(TEXTURE) || defined(TEXTURE_ALPHA_ONLY)
out vec2 oTexcoord;
#endif

#ifdef VERTEX_COLOR
in vec4 aColor;
out vec4 oColor;
#endif

void main() {
#if defined(TEXTURE) || defined(TEXTURE_ALPHA_ONLY)
	oTexcoord = (uTextureMatrix * vec4(aTexcoord0, 0.0, 1.0)).xy;
#endif

#ifdef VERTEX_COLOR
	oColor = aColor;
	
	#ifdef PREMUL_ALPHA
		oColor.rgb *= oColor.a;
	#endif
#endif

    gl_Position = uModelViewProjection * aPosition;
}