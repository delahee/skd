//! #version 140
#include "shaders/common/platform.glsl" //! #include "../common/platform.glsl"
#include "shaders/common/utils.glsl" //! #include "../common/utils.glsl"

in vec4 aPosition;

BEGIN_UNIFORM_BLOCK
UNIFORM mat4 uModel;
UNIFORM mat4 uModelViewProjection;
UNIFORM mat4 uInverseView;
UNIFORM float uT;
END_UNIFORM_BLOCK

out vec4 oLocalPos;
out vec4 oWorldNormal;
out vec4 oViewDir;

void main() {
	oLocalPos = vec4(aPosition.xyz, 1.0);
	oWorldNormal = normalize(uModel * vec4(aPosition.xyz, 0.0)); // since I use sphere centered at 0 I can use local pos as normal
	oViewDir = vec4(uInverseView[2][0], uInverseView[2][1], uInverseView[2][2], uInverseView[2][3]);

	// noise deform
	/*oLocalPos.x += (noise(aPosition.y + uTime * 11.0) - 0.5) * 0.25;
	oLocalPos.y += (noise(aPosition.z + uTime * 7.0) - 0.5) * 0.25;
	oLocalPos.z += (noise(aPosition.x + uTime * 17.0) - 0.5) * 0.25;*/

    gl_Position = uModelViewProjection * oLocalPos;
}