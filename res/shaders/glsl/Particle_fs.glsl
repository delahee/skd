//! #version 430

in vec2 oTexcoord;
in vec4 oColor;

uniform sampler2D uTexture0;

out vec4 fragColor;

void main() {
	vec4 texColor = texture(uTexture0, oTexcoord);
	
	if (texColor.a <= 0.0)
		discard;

	fragColor = texColor * oColor;
}