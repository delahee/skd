out vec4 fragColor;
in vec2 oTexcoord;
in vec2 vTexcoord;
in vec2 viewportXY;
uniform sampler2D uTexture0;
uniform vec2 uTexResolution0;
uniform vec2 uInvTexResolution0;

uniform float charPos[6];

const float offset = 1.0 / 4096.0;

float find_closest(int x, int y, float c0)
{

float dither[64] = float[64](
	00.0, 32.0, 08.0, 40.0, 02.0, 34.0, 10.0, 42.0, /* 8x8 Bayer ordered dithering */
	48.0, 16.0, 56.0, 24.0, 50.0, 18.0, 58.0, 26.0, /* pattern. Each input pixel */
	12.0, 44.0, 04.0, 36.0, 14.0, 46.0, 06.0, 38.0, /* is scaled to the 0..63 range */
	60.0, 28.0, 52.0, 20.0, 62.0, 30.0, 54.0, 22.0, /* before looking in this table */
	03.0, 35.0, 11.0, 43.0, 01.0, 33.0, 09.0, 41.0, /* to determine the action. */
	51.0, 19.0, 59.0, 27.0, 49.0, 17.0, 57.0, 25.0,
	15.0, 47.0, 07.0, 39.0, 13.0, 45.0, 05.0, 37.0,
	63.0, 31.0, 55.0, 23.0, 61.0, 29.0, 53.0, 21.0 );

float limit = 0.0;
if(x < 8)
{
limit = (dither[x+8*y]+1.0)/64.0;
}


if(c0 < limit)
return 0.0;
return 1.0;
}

void main() {

	
	vec2 uv = oTexcoord;
	vec4 tex = texture(uTexture0, uv);
	
	vec4 lum = vec4(0.299, 0.587, 0.114, 0.0);
    float grayscale = dot(tex, lum);
	vec2 xy = gl_FragCoord.xy * .5;
    int x = int(mod(xy.x, 8.0));
    int y = int(mod(xy.y, 8.0));
	
	if (tex.a > 0.5){
		float pos = abs(viewportXY.x)*1.5;
		tex.a*=pos;
		float final = find_closest(x, y, tex.a);
		if (final < 0.001)
				discard;
		fragColor = vec4(tex.xyz, final);
	}
		
	else {
	float a = texture2D(uTexture0, vec2(vTexcoord.x + offset, vTexcoord.y)).a +
			texture2D(uTexture0, vec2(vTexcoord.x, vTexcoord.y - offset)).a +
			texture2D(uTexture0, vec2(vTexcoord.x - offset, vTexcoord.y)).a +
			texture2D(uTexture0, vec2(vTexcoord.x, vTexcoord.y + offset)).a;
		if (tex.a < 1.0 && a > 0.0)
			fragColor = vec4(0.0, 0.0, 0.0, 0.8*(abs(viewportXY.x)+0.1)*2.0);
		else{
			float pos =(abs(viewportXY.x)+0.5)/2.0;
			tex.a*=pos;
			float final = find_closest(x, y, tex.a);
			if (final < 0.001)
				discard;
			fragColor = vec4(tex.xyz, final);
			}
	}
	
}