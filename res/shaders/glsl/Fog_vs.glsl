//! #version 140
#include "shaders/common/platform.glsl" //! #include "../common/platform.glsl"

in vec4 aPosition;
in vec2 aTexcoord0;

BEGIN_UNIFORM_BLOCK
UNIFORM mat4 uModel;
UNIFORM mat4 uModelViewProjection;
UNIFORM mat4 uTextureMatrix;
UNIFORM float uTime;
END_UNIFORM_BLOCK

out vec4 oLocalPosition;
out vec2 oTexcoord;

void main() {
	oTexcoord = (uTextureMatrix * vec4(aTexcoord0, 0.0, 1.0)).xy;
	oLocalPosition = vec4(aPosition.xyz, 1.0) * 100;
    gl_Position = uModelViewProjection * aPosition;
}