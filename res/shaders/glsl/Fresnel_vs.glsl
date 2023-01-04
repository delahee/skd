//! #version 140
#include "shaders/common/platform.glsl" //! #include "../common/platform.glsl"

in vec4 aPosition;

BEGIN_UNIFORM_BLOCK
UNIFORM mat4 uModel;
UNIFORM mat4 uModelViewProjection;
UNIFORM mat4 uInverseView;
UNIFORM float uTime;
END_UNIFORM_BLOCK

out vec4 oWorldNormal;
out vec4 oViewDir;

void main() {
	oWorldNormal = normalize(uModel * vec4(aPosition.xyz, 0.0)); // since I use sphere centered at 0 I can use local pos as normal
	oViewDir = vec4(uInverseView[2][0], uInverseView[2][1], uInverseView[2][2], uInverseView[2][3]);

    gl_Position = uModelViewProjection * aPosition;
}