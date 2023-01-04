#ifndef _HBC_GL_PARTICLE_SYSTEM
#define _HBC_GL_PARTICLE_SYSTEM

#define PI 3.1415926538

#define VM_GLOBAL 0.0
#define VM_FROM_CENTER 1.0

#define EM_POINT 0.0
#define EM_BOX 1.0
#define EM_INSIDE_SPHERE 2.0
#define EM_OUTLINE_SPHERE 3.0
#define EM_CYLINDER 4.0
#define EM_SPRITE 5.0

#define PSM_DISABLED 0.0
#define PSM_LOOP 1.0
#define PSM_ONESHOT 2.0

#define CM_OFF 0.0
#define CM_X_PLAN 1.0
#define CM_NEG_X_PLAN 2.0
#define CM_Y_PLAN 3.0
#define CM_NEG_Y_PLAN 4.0
#define CM_Z_PLAN 5.0
#define CM_NEG_Z_PLAN 6.0

BEGIN_UNIFORM_BLOCK
UNIFORM float uGravityScale;
UNIFORM vec4 uTime;							// x: rs::Timer::dt 			y: rs::Timer::now			z: timescale				w: reset asked
UNIFORM vec4 uEmitTiming;					// x: mode						y: duration					z: delay					w: unused
UNIFORM vec4 uEmitData;						// x: quantity					y: time between emission	z: emitter mode				w: velocity mode
UNIFORM vec4 uInitialPos;					// xyz: emitter position																w: base rotation
UNIFORM vec4 uInitialPosRandom;				// xyz: emitter position size OR EM_SPRITE offset										w: rotation random range

UNIFORM vec4 uInitialVel;					// xyz: base xyz velocity (or x: center velocity y: / z: Z velocity (cylinder only))	w: base rotation speed
UNIFORM vec4 uInitialVelRandom;				// xyz: velocity random range (or same as above but for range)							w: rotation speed random range
UNIFORM vec4 uDamping;						// x:   damping 				yz: unused												w: rotation damping
UNIFORM vec4 uInitialScaleLifetime;			// xy:  base scale											z: base lifetime			w: base delay
UNIFORM vec4 uInitialScaleLifetimeRandom;	// xy:  scale random range									z: lifetime	random range	w: delay range
UNIFORM vec4 uAttractor;					// xyz: attractor pos																	w: attractor force
UNIFORM vec4 uCollision;					// x:   collision pos			y: unused					z: damping					w: collision mode
UNIFORM vec4 uSpriteInfo;					// xy:  width/height			                            z: random pixel             w: unused
UNIFORM vec4 uSpriteUV;						// xyzw: U1 V1 U2 V2
UNIFORM vec4 uNoise;						// xyz: noise intensity for each axis													w: noise speed
END_UNIFORM_BLOCK

#define DeltaTime uTime.x
#define NowTime uTime.y
#define Timescale uTime.z
#define ResetAsked uTime.w

#define PSMode uEmitTiming.x
#define PSDuration uEmitTiming.y
#define PSDelay uEmitTiming.z
// uEmitTiming.w unused

#define EmitQuantity uEmitData.x
#define TimeBetweenEmit uEmitData.y
#define EmitterMode uEmitData.z
#define VelocityMode uEmitData.w

#define InitialPosition uInitialPos.xyz
#define InitialRotation uInitialPos.w
#define InitialPosRot uInitialPos

#define RangePosRot uInitialPosRandom
#define SpriteOffset uInitialPosRandom.xyz // EM_SPRITE only
// uInitialPosRandom.w unused with EM_SPRITE

//uInitialVel
//uInitialVelRandom

#define LinearDamping uDamping.x
// uDamping.y unused
// uDamping.z unused
#define AngularDamping uDamping.w

#define InitialScale uInitialScaleLifetime.xyz
#define RangeScale uInitialScaleLifetimeRandom.xyz
#define InitialLifetime uInitialScaleLifetime.w
#define RangeLifetime uInitialScaleLifetimeRandom.w

#define AttractorPos uAttractor.xyz
#define AttractorIntensity uAttractor.w

#define CollisionPos uCollision.x
// uCollision.y unused
#define CollisionDamping uCollision.z
#define CollisionMode uCollision.w

//uSpriteInfo
//uSpriteUV

#define NoiseIntensityX uNoise.x
#define NoiseIntensityY uNoise.y
#define NoiseIntensityZ uNoise.z
#define NoiseSpeed uNoise.w

#ifdef HAS_TEXTURE
uniform sampler2D uTexture0;
#endif

float rand(float n) { return fract(sin(n) * 43758.5453123); }
float rand_vec2 (vec2 st) {
    return fract(sin(dot(st.xy, vec2(12.9898,78.233))) * 43758.5453123);
}
vec4 rand_vec4(float n) {
	return vec4(
		rand(n),
		rand(n + 0.01),
		rand(n + 0.02),
		rand(n + 0.03)
	);
}

vec3 rand_sphere(float n) {
	float u = rand(n);
    float v = rand(n + 0.01);
    float theta = u * 2.0 * PI;
    float phi = acos(2.0 * v - 1.0);
    float r = pow(rand(n + 0.02), 0.33333333);
    float sinTheta = sin(theta);
    float cosTheta = cos(theta);
    float sinPhi = sin(phi);
    float cosPhi = cos(phi);
	vec3 res;
    res.x = r * sinPhi * cosTheta;
    res.y = r * sinPhi * sinTheta;
    res.z = r * cosPhi;
    return res;
}

float noise (float p){
	float fl = floor(p);
	float fc = fract(p);
	return mix(rand(fl), rand(fl + 1.0), fc);
}
// from https://www.shadertoy.com/view/4dS3Wd
float noise_vec2 (vec2 x) {
    vec2 i = floor(x);
    vec2 f = fract(x);

	// Four corners in 2D of a tile
	float a = rand_vec2(i);
    float b = rand_vec2(i + vec2(1.0, 0.0));
    float c = rand_vec2(i + vec2(0.0, 1.0));
    float d = rand_vec2(i + vec2(1.0, 1.0));

    // Simple 2D lerp using smoothstep envelope between the values.
	// return vec3(mix(mix(a, b, smoothstep(0.0, 1.0, f.x)),
	//			mix(c, d, smoothstep(0.0, 1.0, f.x)),
	//			smoothstep(0.0, 1.0, f.y)));

	// Same code, with the clamps in smoothstep and common subexpressions
	// optimized away.
    vec2 u = f * f * (3.0 - 2.0 * f);
	return mix(a, b, u.x) + (c - a) * u.y * (1.0 - u.x) + (d - b) * u.x * u.y;
}

#endif // _HBC_GL_PARTICLE_SYSTEM