//! #version 140
#include "shaders/common/platform.glsl" //! #include "../common/platform.glsl"

in vec4 aPosition;
in vec4 aColor;

BEGIN_UNIFORM_BLOCK
UNIFORM mat4 uModelViewProjection;
UNIFORM float uTime;
END_UNIFORM_BLOCK

out vec4 oColor;

void main() {
	oColor = aColor;

    gl_Position = uModelViewProjection * aPosition;
}