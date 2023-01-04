//! #version 140
#include "shaders/common/platform.glsl" //! #include "../common/platform.glsl"

in vec2 oTexcoord;

uniform sampler2D uTexture0;
#ifdef PYRAMID_UPSAMPLE
uniform sampler2D uTexture1;
#endif

BEGIN_UNIFORM_BLOCK
UNIFORM vec2 uInvTexResolution0;
#ifdef PYRAMID_THRESHOLD
UNIFORM vec4 uThreshold; // x: threshold value (linear), y: threshold - knee, z: knee * 2, w: 0.25 / knee
#endif
END_UNIFORM_BLOCK

#include "shaders/common/bloom.glsl" //! #include "../common/bloom.glsl"

out vec4 fragColor;

void main() {
	#if defined(PYRAMID_THRESHOLD)
        vec4 down = DownsampleBox13Tap(oTexcoord, uInvTexResolution0);
        fragColor = QuadraticThreshold(down, uThreshold.x, uThreshold.yzw);
	#elif defined(PYRAMID_DOWNSAMPLE)
		vec4 down = DownsampleBox13Tap(oTexcoord, uInvTexResolution0);
		fragColor = down;
	#elif defined(PYRAMID_UPSAMPLE)
		vec4 bloom = UpsampleTent(uTexture0, oTexcoord, uInvTexResolution0);
        vec4 color = texture2D(uTexture1, oTexcoord);
		fragColor = bloom + color;
	#else
		fragColor = vec4(1.0,0,1.0,1.0); // error
	#endif
}