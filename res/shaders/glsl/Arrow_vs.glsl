//! #version 140
#include "shaders/common/platform.glsl" //! #include "../common/platform.glsl"

in vec4 aPosition;

BEGIN_UNIFORM_BLOCK
UNIFORM mat4 uModel;
UNIFORM mat4 uModelViewProjection;
UNIFORM float uTime;
END_UNIFORM_BLOCK

out vec4 oLocalPos;

void main() {
	oLocalPos = aPosition;
	
    vec4 localPos = vec4(aPosition.xyz, 1.0);
    if(gl_VertexID < 24)
        localPos.x += fract(uTime) * 40;

    float posX = (localPos.x + 10.0) / 160.0;
    float height = 200.0 * posX * posX - 200.0 * posX;
    localPos.y += height;

    gl_Position = uModelViewProjection * localPos;
}