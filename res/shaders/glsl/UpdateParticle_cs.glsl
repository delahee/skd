//! #version 430 // to avoid Visual screaming at me
//! #define PASTA_LOCAL_SIZE_X 1024
//! #define MAX_PARTICLES 1024
#include "shaders/common/platform.glsl" //! #include "../common/platform.glsl"
#include "shaders/common/particlesystem.glsl" //! #include "../common/particlesystem.glsl"

layout( std430 ) buffer uRWBuffer0 {
	vec4 global;							// x:   time					y: last emit time			zw: unused
	vec4 pos[MAX_PARTICLES];				// xyz: position																		w: rotation
	vec4 vel[MAX_PARTICLES];				// xyz: velocity																		w: angularVelocity
	vec4 scaleLifetime[MAX_PARTICLES];		// xy:  scale												z:  initial lifetime		w: current lifetime
#ifdef HAS_COLOR_DATA
	vec4 color[MAX_PARTICLES];				// rgba
#endif
};

shared uint ParticleEmittedThisFrameThisGroup;

layout (local_size_x = PASTA_LOCAL_SIZE_X, local_size_y = 1, local_size_z = 1) in;

#ifdef HAS_TEXTURE
bool init_sprite(uint particleId) {
	float randomSeed = gl_GlobalInvocationID.x + NowTime / 64.526;
	if (particleId >= uSpriteInfo.x * uSpriteInfo.y) return false;
	if (rand(randomSeed) > uSpriteInfo.z) return false;

	vec2 pixelPos = vec2(mod(particleId, uSpriteInfo.x) / uSpriteInfo.x, float(particleId / int(uSpriteInfo.x)) / uSpriteInfo.y);
	vec2 uv;
	uv.x = mix(uSpriteUV.x, uSpriteUV.z, pixelPos.x);
	uv.y = mix(uSpriteUV.w, uSpriteUV.y, pixelPos.y);
	vec4 col = texture(uTexture0, uv);
	if (col.a < 0.01) return false;

	float hPos = mod(particleId, uSpriteInfo.x) / 36.0;
	pos[particleId].x = hPos;
	pos[particleId].y = hPos * -1; // for some wacky reason if I put -hPos instead of hPos * -1 the result is not correct on AMD ???
	pos[particleId].z = float(particleId / int(uSpriteInfo.x)) / 9.0;
	pos[particleId].w = 0;

	pos[particleId].xyz += InitialPosition;
	pos[particleId].xyz += SpriteOffset;

#ifdef HAS_COLOR_DATA
	color[particleId].rgba = col;
#endif
	return true;
}
#endif

void init(uint particleId) {
	float randomSeed = gl_GlobalInvocationID.x + NowTime / 64.526;

#ifdef HAS_TEXTURE
	if (EmitterMode == EM_SPRITE) {
		bool emitted = init_sprite(particleId);
		if (!emitted) return;
	} else {
#endif
	pos[particleId] = InitialPosRot;
	vec4 rndPos;
	if (EmitterMode == EM_INSIDE_SPHERE || EmitterMode == EM_OUTLINE_SPHERE || EmitterMode == EM_CYLINDER) {
		rndPos = vec4(rand_sphere(randomSeed), rand(randomSeed + 0.03));
		if (EmitterMode == EM_OUTLINE_SPHERE) rndPos.xyz /= length(rndPos.xyz);
		if (EmitterMode == EM_CYLINDER) {
			rndPos.xy /= length(rndPos.xy);
			rndPos.z = rand(randomSeed + 0.04) * 2.0 - 1.0;
		}
		rndPos.xyz /= 2.0;
	} else {
		rndPos = rand_vec4(randomSeed);
		rndPos.xyz -= 0.5;
	}
	pos[particleId] += rndPos * RangePosRot;

#ifdef HAS_TEXTURE
	}
#endif

	if (VelocityMode == VM_GLOBAL) {
		vel[particleId] = uInitialVel;
		vel[particleId] += rand_vec4(randomSeed + 0.1) * uInitialVelRandom;
	} else if (VelocityMode == VM_FROM_CENTER) {
		vec3 dir;
		if (EmitterMode == EM_CYLINDER) dir = vec3(normalize(pos[particleId].xy - InitialPosRot.xy), 1);
		else dir = normalize(pos[particleId].xyz - InitialPosition);
		vel[particleId].xyz = dir * uInitialVel.x;
		if (EmitterMode == EM_CYLINDER) vel[particleId].z = uInitialVel.z;
		vel[particleId].xyz += dir * rand_vec4(randomSeed + 0.1).xyz * uInitialVelRandom.x;
		if (EmitterMode == EM_CYLINDER) vel[particleId].z += rand(randomSeed + 0.12) * uInitialVelRandom.z;

		vel[particleId].w = uInitialVel.w + rand(randomSeed + 0.13) * uInitialVelRandom.w;
	}

	scaleLifetime[particleId].xyz = InitialScale;
	scaleLifetime[particleId].xyz += rand_vec4(randomSeed + 0.2).xyz * RangeScale;
	scaleLifetime[particleId].w = scaleLifetime[particleId].z + InitialLifetime + rand(randomSeed + 0.23) * RangeLifetime;
}

void update(uint particleId) {
	float dt = DeltaTime * Timescale;
	scaleLifetime[particleId].w -= dt;
	if (scaleLifetime[particleId].w > scaleLifetime[particleId].z) return; // to add a delay per particle

	vec3 acc = vec3(0.0, 0.0, -9.81 * uGravityScale);

	if (AttractorIntensity != 0.0) {
		vec3 dirAttractor = AttractorPos - pos[particleId].xyz;
		float dist = length(dirAttractor);
		if (dist > 0.0) acc += normalize(dirAttractor) * AttractorIntensity / (dist * dist);
	}

	vec3 newPos = pos[particleId].xyz + vel[particleId].xyz * dt + acc * dt * dt * 0.5;
	if (CollisionMode != CM_OFF) { // maybe remove the 2,4,6 collision mode & check the emitter position relative to the plan instead
		     if(CollisionMode == CM_X_PLAN     && newPos.x < CollisionPos) vel[particleId].xyz *= vec3(-1, 1, 1) * CollisionDamping;
		else if(CollisionMode == CM_NEG_X_PLAN && newPos.x > CollisionPos) vel[particleId].xyz *= vec3(-1, 1, 1) * CollisionDamping;

		else if(CollisionMode == CM_Y_PLAN     && newPos.y < CollisionPos) vel[particleId].xyz *= vec3(1, -1, 1) * CollisionDamping;
		else if(CollisionMode == CM_NEG_Y_PLAN && newPos.y > CollisionPos) vel[particleId].xyz *= vec3(1, -1, 1) * CollisionDamping;

		else if(CollisionMode == CM_Z_PLAN     && newPos.z < CollisionPos) vel[particleId].xyz *= vec3(1, 1, -1) * CollisionDamping;
		else if(CollisionMode == CM_NEG_Z_PLAN && newPos.z > CollisionPos) vel[particleId].xyz *= vec3(1, 1, -1) * CollisionDamping;

		pos[particleId].xyz += vel[particleId].xyz * dt + acc * dt * dt * 0.5;
	} else pos[particleId].xyz = newPos;
	pos[particleId].w += vel[particleId].w * dt; // angular rotation
	vel[particleId].xyz += acc * dt;
	vel[particleId].xyz *= pow(1.0 - LinearDamping, dt);
	vel[particleId].w *= pow(1.0 - AngularDamping, dt);
	
	// noise
	pos[particleId].x += (noise_vec2(vec2(float(particleId+0)*PI, NowTime * NoiseSpeed)) - 0.5) * NoiseIntensityX * dt;
	pos[particleId].y += (noise_vec2(vec2(float(particleId+1)*PI, NowTime * NoiseSpeed)) - 0.5) * NoiseIntensityY * dt;
	pos[particleId].z += (noise_vec2(vec2(float(particleId+2)*PI, NowTime * NoiseSpeed)) - 0.5) * NoiseIntensityZ * dt;
}

void main() {
	uint particleId	= gl_GlobalInvocationID.x;
	float dt = DeltaTime * Timescale;

	int toEmit;
	if (EmitterMode == EM_SPRITE)
		toEmit = int(MAX_PARTICLES);
	else
		toEmit = int(EmitQuantity);

	// we divide evenly the amount of particle to emit for each group
	int toEmitGroup = int(toEmit / gl_NumWorkGroups.x);
	int remaining = int(mod(toEmit, gl_NumWorkGroups.x));
	if (remaining > 0 && gl_WorkGroupID.x < remaining) toEmitGroup++; // TODO: maybe spreadout the remaining amount randomly to avoid saturating the firsts groups?

	if (gl_LocalInvocationID.x == 0) ParticleEmittedThisFrameThisGroup = 0; // each group emit toEmitGroup particles, the shared variable ParticleEmittedThisFrameThisGroup keep track the amount already instanticiate for the current group
	barrier(); // it's the first local invoc who set the ParticleEmittedThisFrameThisGroup, we wait here for each group to sync
	
	float currentTime = global.x - PSDelay;
	bool shouldEmitNewParticle = currentTime >= 0.0 && (global.x - global.y) > TimeBetweenEmit && PSMode != PSM_DISABLED;
	if (PSMode == PSM_ONESHOT) shouldEmitNewParticle = shouldEmitNewParticle && currentTime < PSDuration;
	barrier();
	if (particleId == (MAX_PARTICLES - 1)) { // update global only with the very last particle after all global read to avoid writing conflict
		global.x += dt;
		if(ResetAsked == 1.0) {
			global.x = 0.0; global.y = -999999.0;
		}
	}
	if (ResetAsked == 1.0) {
		shouldEmitNewParticle = false;
		scaleLifetime[particleId].w = 0.0;
	}
	if (gl_LocalInvocationID.x == 0) {
		if (shouldEmitNewParticle) global.y = global.x; // set the global timer for particule emission
	}

	if (scaleLifetime[particleId].w <= 0.0 && shouldEmitNewParticle) {
		// if the current particle is dead and we shouldEmitNewParticle this frame we check if this group can still initiate a new particle:
		if(atomicAdd(ParticleEmittedThisFrameThisGroup, 1) < toEmitGroup) init(particleId);
	} else {
		// this is the update portion for living particles:
		update(particleId);
	}
}