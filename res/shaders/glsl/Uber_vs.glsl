//! #version 140
#include "shaders/common/platform.glsl" //! #include "../common/platform.glsl"
#include "shaders/common/utils.glsl" //! #include "../common/utils.glsl"

//! #define VERTEX_COLOR
//! #define TEXTURE
//! #define PREMUL_ALPHA

//! #define HAS_FXAA
//! #define HAS_DITHERING

in vec4 aPosition;
in vec2 aTexcoord0;

BEGIN_UNIFORM_BLOCK
UNIFORM mat4 uModelViewProjection;
UNIFORM mat4 uTextureMatrix;
#ifdef TEXTURE
UNIFORM vec2 uTexResolution0;
UNIFORM vec2 uInvTexResolution0;
#endif
END_UNIFORM_BLOCK

#ifdef VERTEX_COLOR
in vec4 aColor;
out vec4 oColor;
#endif

#ifdef TEXTURE
out vec2 oTexcoord;
out vec2 vTexcoord;
#endif

#ifdef HAS_FXAA
out lowp vec2 oFxaaNW;
out lowp vec2 oFxaaNE;
out lowp vec2 oFxaaSE;
out lowp vec2 oFxaaSW;
#endif

#ifdef HAS_DITHERING
out vec2 oViewportXY;
#endif

void main()
{
#ifdef TEXTURE
	oTexcoord = (uTextureMatrix * vec4(aTexcoord0, 0.0, 1.0)).xy;
	vTexcoord = aTexcoord0;
#endif

#ifdef VERTEX_COLOR
	oColor = aColor;
	
	#ifdef PREMUL_ALPHA
		oColor.rgb *= oColor.a;
	#endif
#endif

#ifdef HAS_FXAA 
	vec2 texRes = uInvTexResolution0;
	oFxaaNW = oTexcoord + vec2(-texRes.x, 	-texRes.y);
	oFxaaNE = oTexcoord + vec2( texRes.x, 	-texRes.y);
	oFxaaSW = oTexcoord + vec2(-texRes.x, 	 texRes.y);
	oFxaaSE = oTexcoord + vec2( texRes.x, 	 texRes.y);
#endif

    gl_Position = uModelViewProjection * aPosition;
	
#ifdef HAS_DITHERING
	oViewportXY = gl_Position.xy;
#endif
}