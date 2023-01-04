in vec4 aPosition;
in vec2 aTexcoord0;
in vec4 aColor;


uniform mat4 uModelViewProjection;
uniform mat4 uTextureMatrix;
out vec2 oTexcoord;
out vec2 vTexcoord;
out vec2 viewportXY;

void main() {
	oTexcoord = (uTextureMatrix * vec4(aTexcoord0, 0.0, 1.0)).xy;
	vTexcoord = aTexcoord0;

    gl_Position = uModelViewProjection * aPosition;
	viewportXY = gl_Position.xy;
}