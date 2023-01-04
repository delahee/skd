//! #version 140
#include "shaders/common/platform.glsl" //! #include "../common/platform.glsl"
#include "shaders/common/utils.glsl" //! #include "../common/utils.glsl"

in vec2 oTexcoord;

uniform sampler2D uTexture0;
BEGIN_UNIFORM_BLOCK
UNIFORM vec4 uCameraParams;
UNIFORM float uTime;
END_UNIFORM_BLOCK

out vec4 fragColor;

void main() {
	vec4 texColor = texture2D(uTexture0, oTexcoord);
	
	if( texColor.a <= 0.0)
		discard;

	/*vec2 xy = gl_FragCoord.xy * 0.5;
	int x = int(mod(xy.x, 6));
	int y = int(
		mod(xy.y
		//+ floor(uTime * 7) * 2
		, 3)
	);

	float scan = mod((y - x / 2), 3) / 3.0;
	if(scan == 0) discard;*/

	vec2 pos = (gl_FragCoord.xy / uCameraParams.zw) - uCameraParams.xy;
	pos = floor(pos);

	float scan = step(fract(pos.y * 0.2 + uTime * 2.0 + noise(pos.y + uTime * 5) * 0.5), 0.2) * 0.65;
	fragColor = vec4(1.0, 0.4, 0.88, 0.6) * vec4(texColor.rgb * 4.0, texColor.a) + scan;

	/*vec2 size = 1 / vec2(4,4);
    float total = floor(gl_FragCoord.x*float(size.x)) +
                  floor(gl_FragCoord.y*float(size.y));
    bool isEven = mod(total,2.0)==0.0;
    vec4 col1 = vec4(0, 0, 0, 0);
    vec4 col2 = vec4(texColor.rgb * 4,texColor.a) * vec4(1, 0.4, 0.88, 0.6);

    vec4 col = ((isEven)? col1:col2);
	if(col.a <= 0.0)
		discard;*/
    //fragColor = vec4(texColor.rgb * 0.33, texColor.a); //vec4(scan,scan,scan,1);
}