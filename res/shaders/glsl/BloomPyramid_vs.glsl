//! #version 140
#include "shaders/common/platform.glsl" //! #include "../common/platform.glsl"

in vec4 aPosition;
in vec2 aTexcoord0;

BEGIN_UNIFORM_BLOCK
UNIFORM mat4 uModelViewProjection;
UNIFORM mat4 uTextureMatrix;
END_UNIFORM_BLOCK

out vec2 oTexcoord;

void main() {
	oTexcoord = (uTextureMatrix * vec4(aTexcoord0, 0.0, 1.0)).xy;

    gl_Position = uModelViewProjection * aPosition;
}