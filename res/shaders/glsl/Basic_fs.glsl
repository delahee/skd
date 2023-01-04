//! #version 140
#include "shaders/common/platform.glsl" //! #include "../common/platform.glsl"

#ifdef VERTEX_COLOR
in vec4 oColor;
#endif

#if defined(TEXTURE) || defined(TEXTURE_ALPHA_ONLY)
in vec2 oTexcoord;
uniform sampler2D uTexture0;
#endif

#ifdef UNIFORM_COLOR
BEGIN_UNIFORM_BLOCK
UNIFORM vec4 uColor;
END_UNIFORM_BLOCK
#endif

out vec4 fragColor;

void main() {
	if (false) { //depth block
		float depth = gl_FragCoord.z / gl_FragCoord.w;
		#if defined(UNIFORM_COLOR)
		fragColor = vec4(depth,depth,depth,1.0) + uColor * 0.00001;
		#else
		fragColor = vec4(depth,depth,depth,1.0);
		#endif
		return;
	}
	
	if (false) { //show alpha block
		#ifdef TEXTURE
			vec4 texColor = texture2D(uTexture0, oTexcoord);
			#if defined(UNIFORM_COLOR)
			fragColor = vec4(texColor.a,0,texColor.a,1) + uColor * 0.00001;
			#else
			fragColor = vec4(texColor.a,0,texColor.a,1);
			#endif
			return;
		#endif
	}

#ifdef TEXTURE
	vec4 color = texture2D(uTexture0, oTexcoord);
#else
	vec4 color = vec4(1.0, 1.0, 1.0, 1.0);
#ifdef TEXTURE_ALPHA_ONLY
	color.a *= texture2D(uTexture0, oTexcoord).a;
#endif
#endif
	
#ifdef VERTEX_COLOR
	color *= oColor;
#endif

#ifdef UNIFORM_COLOR
	color *= uColor;
#endif

#ifdef KILL_ALPHA
	if (color.a <= 0.0)
		discard;
#endif
	
	fragColor = color;
}