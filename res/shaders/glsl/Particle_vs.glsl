//! #version 430
//! #define MAX_PARTICLES 512
#include "shaders/common/platform.glsl" //! #include "../common/platform.glsl"
#define PI 3.1415926538
#define RADIUS 18
#define HALF_RADIUS 9

in vec4 aPosition;
in vec2 aTexcoord0;

BEGIN_UNIFORM_BLOCK
UNIFORM mat4 uModelViewProjection;
UNIFORM mat4 uTextureMatrix;
UNIFORM vec4 uColor;

UNIFORM vec4 uColors[4];
UNIFORM vec4 uColorsOffsets;				// xyzw: offset for color transition (value normalized)
UNIFORM vec4 uPivot;						// xy: pivot												zw: unused
UNIFORM vec4 uTileUV;
UNIFORM vec4 uAnimSheet;
END_UNIFORM_BLOCK

out vec2 oTexcoord;
out vec4 oColor;
out gl_PerVertex {
    vec4 gl_Position;
};

layout( std430 ) buffer vsRWBuffer0 {
	vec4 global;							// x:   time					y: last emit time			zw: unused
	vec4 pos[MAX_PARTICLES];				// xyz: position																		w: rotation
	vec4 vel[MAX_PARTICLES];				// xyz: velocity																		w: angularVelocity
	vec4 scaleLifetime[MAX_PARTICLES];		// xy:  scale												z:  initial lifetime		w: current lifetime
#ifdef HAS_COLOR_DATA
	vec4 color[MAX_PARTICLES];				// rgba
#endif
};

// https://gist.github.com/JensAyton/499357
vec3 hsv2rgb(float h, float s, float v) {
	float hi = h * 3.0 / PI;  // Sector, [-3..3)
	float f = hi - floor(hi);  // Fractional part.
    
	vec4 components = vec4(
		0.0,
		s,
		s * f,
		s * (1.0 - f)
	);
	
	components = (1.0 - components) * v;
 
	return (hi < 0.0) ? (
			(hi < -2.0) ? components.xwy :
			(hi < -1.0) ? components.zxy :
			components.yxw
		) : (
			(hi < 1.0) ? components.yzx :
			(hi < 2.0) ? components.wyx :
			components.xyz
		);
}

mat3 rotation3dZ(float angle) {
	float s = sin(angle);
	float c = cos(angle);

	return mat3(
		  c,   s, 0.0,
		 -s,   c, 0.0,
		0.0, 0.0, 1.0
	);
}

void main() {
	float normalizedLifetime = 1 - (scaleLifetime[gl_InstanceID].w / scaleLifetime[gl_InstanceID].z);
	normalizedLifetime = clamp(normalizedLifetime, 0, 1);

	float colorLifetime;

	colorLifetime = (normalizedLifetime - uColorsOffsets.x) / (uColorsOffsets.y - uColorsOffsets.x);
	colorLifetime = clamp(colorLifetime, 0, 1);
	oColor = mix(uColors[0], uColors[1], colorLifetime);
	colorLifetime = (normalizedLifetime - uColorsOffsets.y) / (uColorsOffsets.z - uColorsOffsets.y);
	colorLifetime = clamp(colorLifetime, 0, 1);
	oColor = mix(oColor, uColors[2], colorLifetime);
	colorLifetime = (normalizedLifetime - uColorsOffsets.z) / (uColorsOffsets.w - uColorsOffsets.z);
	colorLifetime = clamp(colorLifetime, 0, 1);
	oColor = mix(oColor, uColors[3], colorLifetime);

#ifdef HAS_COLOR_DATA
	oColor *= color[gl_InstanceID];
#endif
	oColor *= uColor;
	
	vec2 uv = aTexcoord0;
	// spritesheet
	int nbFrame = int(uAnimSheet.x * uAnimSheet.y);
	int currentFrame = int(nbFrame * normalizedLifetime);
	uv.xy = uv.xy / uAnimSheet.xy;
	uv.x += mod(currentFrame, 7) / uAnimSheet.x;
	uv.y += (currentFrame / 7) / uAnimSheet.y;

	uv.x = mix(uTileUV.x, uTileUV.z, uv.x);
	uv.y = mix(uTileUV.y, uTileUV.w, uv.y);

	oTexcoord = (uTextureMatrix * vec4(uv, 0.0, 1.0)).xy;

	vec4 posVertex = aPosition;
	posVertex.xy += uPivot.xy;
	posVertex.x *= scaleLifetime[gl_InstanceID].x * (uPivot.z / uAnimSheet.x);
	posVertex.y *= scaleLifetime[gl_InstanceID].y * (uPivot.w / uAnimSheet.y);
	posVertex.xyz = rotation3dZ(pos[gl_InstanceID].w) * posVertex.xyz;
	if(scaleLifetime[gl_InstanceID].w <= 0.0) posVertex *= 0.0;

	vec3 gridPos;
	gridPos.x = ( pos[gl_InstanceID].x - pos[gl_InstanceID].y) * RADIUS;
	gridPos.y = ( pos[gl_InstanceID].x + pos[gl_InstanceID].y - pos[gl_InstanceID].z) * HALF_RADIUS;
	gridPos.z = (-pos[gl_InstanceID].x - pos[gl_InstanceID].y - pos[gl_InstanceID].z) * HALF_RADIUS;

    gl_Position = uModelViewProjection * vec4(posVertex.xyz + gridPos.xyz, 1);
}