//! #version 140
#include "shaders/common/platform.glsl" //! #include "../common/platform.glsl"

in vec4 aPosition;
in vec4 aColor;
in vec2 aTexcoord0;

BEGIN_UNIFORM_BLOCK
UNIFORM mat4 uModel;
UNIFORM mat4 uModelViewProjection;
UNIFORM mat4 uTextureMatrix;
UNIFORM float uTime;
END_UNIFORM_BLOCK

out vec4 oColor;
out vec2 oTexcoord;
out vec4 oWorldPosition;

void main() {
	oColor = aColor;
	oTexcoord = (uTextureMatrix * vec4(aTexcoord0, 0.0, 1.0)).xy;

	oWorldPosition = uModel * aPosition;
    gl_Position = uModelViewProjection * aPosition;
}