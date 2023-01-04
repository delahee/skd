//! #version 140
// for better error detection in the GLSL Visual extension

//! #define VERTEX_COLOR
//! #define UNIFORM_COLOR
//! #define TEXTURE
//! #define PREMUL_ALPHA
//! #define KILL_ALPHA
//! #define TEXTURE_ALPHA_ONLY

// #define HAS_GBLUR
// #define HAS_BLOOM
// #define HAS_RGB_OFFSET
//! #define HAS_GLITCH
//! #define HAS_DISPLACE
//! #define HAS_COLOR_MATRIX
//! #define HAS_FXAA
//! #define HAS_DISSOLVE
//! #define DISSOLVE_USE_TEXTURE
//! #define RENDER_NOISE
//! #define HAS_DISTORTION
//! #define HAS_CHROMATIC_ABERRATION
//! #define HAS_BLOOM_PYRAMID
//! #define HAS_VIGNETTE
//! #define HAS_COLOR_ADD

//! #define PASTA_FRAGMENT_SHADER
#include "shaders/common/platform.glsl" //! #include "../common/platform.glsl"
#include "shaders/common/utils.glsl" //! #include "../common/utils.glsl"

#ifdef VERTEX_COLOR
in vec4 oColor;
#endif

#ifdef TEXTURE
in vec2 oTexcoord;
in vec2 vTexcoord;

uniform sampler2D uTexture0;
#endif

#if defined(HAS_DISPLACE) || defined(HAS_BLOOM_PYRAMID) || defined(HAS_DISSOLVE)
uniform sampler2D uTexture1;
#endif

BEGIN_UNIFORM_BLOCK
#ifdef UNIFORM_COLOR
	UNIFORM vec4 uColor;
#endif
#ifdef TEXTURE
	UNIFORM vec2 uTexResolution0;
	UNIFORM vec2 uInvTexResolution0;
#endif
	UNIFORM float uTime;


#if defined(HAS_DISPLACE) || defined(HAS_BLOOM_PYRAMID) || defined(HAS_DISSOLVE)
	UNIFORM vec2 uInvTexResolution1;
#endif
#ifdef HAS_DISSOLVE
	UNIFORM float uDissolveProgress;
	UNIFORM float uDissolveZoom;
#endif
#ifdef HAS_DISPLACE
	UNIFORM float uDisplaceAmount;
#endif
#ifdef HAS_COLOR_MATRIX
	UNIFORM mat4 uColorMatrix;
#endif
#ifdef HAS_GLITCH
	UNIFORM vec4 uGlitchParams;
#endif
#ifdef HAS_GBLUR
	UNIFORM int uNbSamples;
	UNIFORM float uKernel[64 * 4];
	UNIFORM vec2 uSampleOffsetsXY[64 * 4];
#endif
#ifdef HAS_RGB_OFFSET
	UNIFORM vec2 uRGBOffset[3];
#endif
#ifdef HAS_BLOOM
	UNIFORM float uPass;
	UNIFORM vec4 uMul;
	UNIFORM vec3 uLuminanceVector;
#endif
#ifdef HAS_BLOOM_PYRAMID
	UNIFORM float uBloomIntensity;
	UNIFORM vec3 uBloomColor;
#endif
#ifdef HAS_DISTORTION
	UNIFORM vec4 uDistortionAmount;
	UNIFORM float uDownloadTransitionAmount;
#endif
#ifdef HAS_VIGNETTE
	UNIFORM vec3 uVignetteAmount;
	UNIFORM vec3 uVignetteColor;
#endif
#ifdef HAS_CHROMATIC_ABERRATION
	UNIFORM vec2 uChromaticAberrationAmount;
#endif
#ifdef HAS_DITHERING
	UNIFORM float uDitherPattern[64];
#endif
#ifdef HAS_COLOR_ADD
	UNIFORM vec4 uColorAdd;
#endif
END_UNIFORM_BLOCK

out vec4 fragColor;

#ifdef HAS_BLOOM_PYRAMID
#include "shaders/common/bloom.glsl" //! #include "../common/bloom.glsl"
#endif

#ifdef HAS_DISSOLVE
	float cnoise(vec2 P) {
		vec4 Pi = floor(P.xyxy) + vec4(0.0, 0.0, 1.0, 1.0);
		vec4 Pf = fract(P.xyxy) - vec4(0.0, 0.0, 1.0, 1.0);
		Pi = mod289(Pi); // To avoid truncation effects in permutation
		vec4 ix = Pi.xzxz;
		vec4 iy = Pi.yyww;
		vec4 fx = Pf.xzxz;
		vec4 fy = Pf.yyww;

		vec4 i = permute(permute(ix) + iy);

		vec4 gx = fract(i * (1.0 / 41.0)) * 2.0 - 1.0 ;
		vec4 gy = abs(gx) - 0.5 ;
		vec4 tx = floor(gx + 0.5);
		gx = gx - tx;

		vec2 g00 = vec2(gx.x,gy.x);
		vec2 g10 = vec2(gx.y,gy.y);
		vec2 g01 = vec2(gx.z,gy.z);
		vec2 g11 = vec2(gx.w,gy.w);

		vec4 norm = taylorInvSqrt(vec4(dot(g00, g00), dot(g01, g01), dot(g10, g10), dot(g11, g11)));
		g00 *= norm.x;  
		g01 *= norm.y;  
		g10 *= norm.z;  
		g11 *= norm.w;  

		float n00 = dot(g00, vec2(fx.x, fy.x));
		float n10 = dot(g10, vec2(fx.y, fy.y));
		float n01 = dot(g01, vec2(fx.z, fy.z));
		float n11 = dot(g11, vec2(fx.w, fy.w));

		vec2 fade_xy = fade(Pf.xy);
		vec2 n_x = mix(vec2(n00, n01), vec2(n10, n11), fade_xy.x);
		float n_xy = mix(n_x.x, n_x.y, fade_xy.y);
		return 2.3 * n_xy;
	}
#endif

#ifdef HAS_GLITCH
	vec2 glitchUV(vec2 uv, vec2 invResolution, float phase) {
		float off = 0.0;
		float line = mix(uv.y, uv.x, uGlitchParams.z);
		float res = mix(uTexResolution0.y, uTexResolution0.x, uGlitchParams.z);
		float size = mix(invResolution.x, invResolution.y, uGlitchParams.z);

		float divider = res / uGlitchParams.w;
		float pos = (floor(line * divider) / divider + phase) * res;
		off += uGlitchParams.x * (noise(pos) - 0.5) * size;

		float dividerBig = res / 20.0;
		float posBig = (floor(line * dividerBig) / dividerBig + phase) * res;
		if(noise(posBig * 5.0) > (1.0 - uGlitchParams.y))
			off += uGlitchParams.x * (noise(posBig * 10.0) - 0.5) * size * 20.0;

		vec2 newUV = uv;
		newUV.x += off * (1.0 - uGlitchParams.z);
		newUV.y += off * uGlitchParams.z;
		return newUV;
	}
#endif

#ifdef HAS_GBLUR
	vec4 blurGauss(sampler2D image, vec2 uv){
		vec4 color = vec4(0.0);
		for (int i = 0; i < uNbSamples; i++) 
			color += texture2D(image, uv + uSampleOffsetsXY[i]) * vec4(uKernel[i]);
		return color;
	}
#endif

#ifdef HAS_RGB_OFFSET
	vec4 sampleWithOffsets(sampler2D image, vec2 uv, vec2 ofs[3]){
		vec4 color = vec4(0.0);
		vec4 off0 = texture2D(image, uv + ofs[0]);
		vec4 off1 = texture2D(image, uv + ofs[1]);
		vec4 off2 = texture2D(image, uv + ofs[2]);

		color.r = off0.r;
		color.g = off1.g;
		color.b = off2.b;
		color.a = max(
		    off0.a,
			max(off1.a, off2.a)
		);
		return color;
	}
#endif

#ifdef HAS_BLOOM
	//takes a samples, high pass and color it
	vec4 bloomPass(vec4 color){
		//return color;
		float 	alpha = color.a;
		
		float	luminance = dot(uLuminanceVector, color.rgb );
		
		luminance = max(0.0, luminance - uPass);
		
		color *= sign(luminance);
		color *= uMul;
		return color;
	}
#endif

#ifdef HAS_DISPLACE
	vec2 displaceUV( vec2 uv, sampler2D displaceImage, float displaceAmount){
		vec2 dir = texture2D(displaceImage, uv ).xy;
		
		uv += (dir * vec2(2.0) - vec2(1.0)) * vec2(displaceAmount);
		
		return uv;
	}
#endif

#ifdef HAS_FXAA 
	in lowp vec2 oFxaaNW;
	in lowp vec2 oFxaaNE;
	in lowp vec2 oFxaaSE;
	in lowp vec2 oFxaaSW;

	vec4 fxaa(sampler2D tex, vec2 uv, vec2 invResolution, vec2 nw, vec2 ne, vec2 sw, vec2 se) {
		float FXAA_REDUCE_MIN = (1.0 / 128.0);
		float FXAA_REDUCE_MUL = 1.0 / 8.0;
		float FXAA_SPAN_MAX = 8.0;
			
		vec3 rgbNW = texture2D(tex, nw).xyz;
		vec3 rgbNE = texture2D(tex, ne).xyz;
		vec3 rgbSW = texture2D(tex, sw).xyz;
		vec3 rgbSE = texture2D(tex, se).xyz;
		vec4 texColor = texture2D(tex, uv);
		vec3 rgbM  = texColor.xyz;
		vec3 luma = vec3(0.299, 0.587, 0.114);
		float lumaNW = dot(rgbNW, luma);
		float lumaNE = dot(rgbNE, luma);
		float lumaSW = dot(rgbSW, luma);
		float lumaSE = dot(rgbSE, luma);
		float lumaM  = dot(rgbM,  luma);
		float lumaMin = min(lumaM, min(min(lumaNW, lumaNE), min(lumaSW, lumaSE)));
		float lumaMax = max(lumaM, max(max(lumaNW, lumaNE), max(lumaSW, lumaSE)));
		mediump vec2 dir;
		dir.x = -((lumaNW + lumaNE) - (lumaSW + lumaSE));
		dir.y =  ((lumaNW + lumaSW) - (lumaNE + lumaSE));
			
		float dirReduce = max((lumaNW + lumaNE + lumaSW + lumaSE) *
							  (0.25 * FXAA_REDUCE_MUL), FXAA_REDUCE_MIN);
			
		float rcpDirMin = 1.0 / (min(abs(dir.x), abs(dir.y)) + dirReduce);
		dir = min(vec2(FXAA_SPAN_MAX, FXAA_SPAN_MAX),
			      max(vec2(-FXAA_SPAN_MAX, -FXAA_SPAN_MAX),
				  dir * rcpDirMin)) * invResolution;
			
		vec3 rgbA = 0.5 * (
			texture2D(tex, uv + dir * (1.0 / 3.0 - 0.5)).xyz +
			texture2D(tex, uv + dir * (2.0 / 3.0 - 0.5)).xyz);
		vec3 rgbB = rgbA * 0.5 + 0.25 * (
			texture2D(tex, uv + dir * -0.5).xyz +
			texture2D(tex, uv + dir * 0.5).xyz);

		float lumaB = dot(rgbB, luma);
			
		vec4 color;
		if ((lumaB < lumaMin) || (lumaB > lumaMax))
			color = vec4(rgbA, texColor.a);
		else
			color = vec4(rgbB, texColor.a);
		return color;
	}
#endif

#ifdef HAS_DISTORTION
	vec2 Distort(vec2 uv) {
		uv = (uv - 0.5) * uDistortionAmount.z + 0.5;
		vec2 ruv = vec2(1, 1) /* intensity X/Y */ * (uv - 0.5 /* center X/Y */);
		float ru = length(ruv);

		if (uDistortionAmount.w > 0.0) {
			float wu = ru * uDistortionAmount.x;
			ru = tan(wu) * (1.0 / (ru * uDistortionAmount.y));
			uv = uv + ruv * (ru - 1.0);
		} else {
			ru = (1.0 / ru) * uDistortionAmount.x * atan(ru * uDistortionAmount.y);
			uv = uv + ruv * (ru - 1.0);
		}

		return uv;
	}
#endif

#ifdef HAS_CHROMATIC_ABERRATION
	vec4 spectralColor(float t) {
		vec4 res = vec4(0.0, 0.0, 0.0, 1.0);
		
		res.r = 1.0 - smoothstep(0.0, 0.6,t);
		res.b = smoothstep(0.4, 1.0, t);
		res.g = 1.0 - res.r - res.b;

		return res;
	}

	//seem to be a problem on nvidia driver here
	// https://twitter.com/pixelmager/status/717019757766123520
	vec4 chromaticAberration(sampler2D image, vec2 uv) {
		int samples = int(uChromaticAberrationAmount.y);
        vec2 pos = uv;
		vec2 coords = 2.0 * pos - 1.0; // [-1; 1]
        vec2 end = pos - coords * dot(coords, coords) * uChromaticAberrationAmount.x;
        vec2 delta = (end - pos) / float(samples);
        vec4 sum = vec4(0.0);
		vec4 filterSum = vec4(0.0);
        for (int i = 0; i < samples; i++) {
			float t = (float(i) + 0.5) / float(samples);
			vec4 s = texture2D(image, pos.xy);
            vec4 spec = spectralColor(t);
            sum += s * spec;
            filterSum += spec;
            pos += delta;
        }
        return sum / filterSum;
	}
#endif

#ifdef HAS_VIGNETTE
	// from https://www.shadertoy.com/view/MslGR8
	// note: valve edition
	//       from http://alex.vlachos.com/graphics/Alex_Vlachos_Advanced_VR_Rendering_GDC2015.pdf
	// note: input in pixels (ie not normalized uv)
	vec3 ScreenSpaceDither( vec2 vScreenPos )
	{
		// Iestyn's RGB dither (7 asm instructions) from Portal 2 X360, slightly modified for VR
		//vec3 vDither = vec3( dot( vec2( 171.0, 231.0 ), vScreenPos.xy + iTime ) );
		vec3 vDither = vec3( dot( vec2( 171.0, 231.0 ), vScreenPos.xy ) );
		vDither.rgb = fract( vDither.rgb / vec3( 103.0, 71.0, 97.0 ) );
		
		//note: apply triangular pdf
		//vDither.r = remap_noise_tri_erp(vDither.r)*2.0-0.5;
		//vDither.g = remap_noise_tri_erp(vDither.g)*2.0-0.5;
		//vDither.b = remap_noise_tri_erp(vDither.b)*2.0-0.5;
		
		return vDither.rgb / 255.0; //note: looks better without 0.375...

		//note: not sure why the 0.5-offset is there...
		//vDither.rgb = fract( vDither.rgb / vec3( 103.0, 71.0, 97.0 ) ) - vec3( 0.5, 0.5, 0.5 );
		//return (vDither.rgb / 255.0) * 0.375;
	}
#endif

#ifdef HAS_DITHERING
	in vec2 oViewportXY;

	const float offset = 1.0 / 4096.0;
	float find_closest(int x, int y, float c0) {	
		float limit = 0.0;
		if(x < 8)
			limit = (uDitherPattern[x+8*y]+1.0)/64.0;	
		if (c0 < limit)
			return 0.0;
		return 1.0;
	}
#endif

void main() {
#if !defined(TEXTURE)
	vec4 texColor = vec4(1.0, 1.0, 1.0, 1.0);
#else
	vec2 uv = oTexcoord;
	
	#if defined(HAS_GBLUR)
		vec4 texColor = blurGauss(uTexture0, uv);
	#elif defined(HAS_BLOOM)
		vec4 texColor = bloomPass(texture2D(uTexture0, uv)); // old bloom
	#else // new post-fx stack
		#ifdef HAS_DISTORTION
			uv = Distort(uv);
			float off = 2 + rand(int(uv.x * uTexResolution0)) * 0.5;
			uv.y -= mix(off, 0, uDownloadTransitionAmount);
		#endif

		#if defined(HAS_DISPLACE)
			uv = displaceUV(uv, uTexture1, uDisplaceAmount);
		#endif
	
		#if defined(HAS_CHROMATIC_ABERRATION)
			vec4 texColor = chromaticAberration(uTexture0, uv);
		#elif defined(HAS_RGB_OFFSET)
			vec4 texColor = sampleWithOffsets(uTexture0, uv, uRGBOffset);
		#elif defined(HAS_FXAA)
			vec4 texColor = fxaa(uTexture0, uv, uInvTexResolution0, oFxaaNW, oFxaaNE, oFxaaSW, oFxaaSE);
		#else
			vec4 texColor = texture2D(uTexture0, uv);
		#endif

		#ifdef KILL_ALPHA
			if (texColor.a <= 0.001)
				discard;
		#endif

		#ifdef HAS_GLITCH
			float phase = mod(uTime, 100.0);
			texColor.r += texture2D(uTexture0, glitchUV(uv, uInvTexResolution0, phase       )).r;
			texColor.g += texture2D(uTexture0, glitchUV(uv, uInvTexResolution0, phase + 33.0)).g;
			texColor.b += texture2D(uTexture0, glitchUV(uv, uInvTexResolution0, phase + 66.0)).b;
			texColor.rgb /= 2.0;
		#endif
		
		//debugDepth(); return;
		//debugAlpha(texColor); return;

		#ifdef HAS_BLOOM_PYRAMID
			vec4 bloom = UpsampleTent(uTexture1, uv, uInvTexResolution1);

			bloom *= uBloomIntensity;
			texColor += bloom * vec4(uBloomColor, 1.0);
		#endif
	
		#ifdef HAS_VIGNETTE
			vec2 d = abs(uv - 0.5) * uVignetteAmount.x;
			d = clamp(d, 0.0, 1.0);
			d = pow(d, uVignetteAmount.zz);
			float vfactor = pow(clamp(1.0 - dot(d, d), 0.0, 1.0), uVignetteAmount.y);
			texColor.rgb *= mix(uVignetteColor, vec3(1.0, 1.0, 1.0), vfactor) + ScreenSpaceDither(uv / uInvTexResolution0) * 5.0;
			texColor.a = mix(1.0, texColor.a, vfactor);
		#endif

		// GRAIN
	#endif
	vec4 finalColor = texColor; 
	
	#ifdef VERTEX_COLOR
		finalColor *= oColor;
	#endif

	#ifdef UNIFORM_COLOR
		finalColor *= uColor;
	#endif

	#ifdef HAS_COLOR_ADD
		finalColor += uColorAdd;
	#endif
	
	#ifdef HAS_COLOR_MATRIX
		finalColor *= uColorMatrix;
	#endif
	
	#ifdef HAS_DITHERING
		vec4 lum = vec4(0.299, 0.587, 0.114, 0.0);
		vec2 xy = gl_FragCoord.xy * .5;
		int x = int(mod(xy.x, 8.0));
		int y = int(mod(xy.y, 8.0));
		
		if (finalColor.a > 0.5) {
			float pos = abs(oViewportXY.x)*1.5;
			finalColor.a = pos;
			float final = find_closest(x, y, finalColor.a);
			if (final < 0.1)
				discard;
			finalColor = vec4(finalColor.xyz, final);
		} else {
			float a = texture2D(uTexture0, vec2(vTexcoord.x + 1.0, vTexcoord.y)).a +
					  texture2D(uTexture0, vec2(vTexcoord.x, vTexcoord.y - 1.0)).a +
					  texture2D(uTexture0, vec2(vTexcoord.x - 1.0, vTexcoord.y)).a +
					  texture2D(uTexture0, vec2(vTexcoord.x, vTexcoord.y + 1.0)).a;
			if (finalColor.a < 1.0 && a >= 0.0)
				finalColor = vec4(0.0, 0.0, 0.0, (abs(oViewportXY.x)+0.1)*1.80);
			else{
				float pos = abs(oViewportXY.x)*1.5;
				finalColor.a = pos;
				float final = find_closest(x, y, finalColor.a);
				if (final < 0.1)
					discard;
				finalColor = vec4(finalColor.xyz, final);
			}
		}
	#endif

	#ifdef HAS_DISSOLVE
		#ifdef DISSOLVE_USE_TEXTURE
			vec4 noise = texture2D(uTexture1, uv/uInvTexResolution0 * uDissolveZoom);
			float value = aaStep(uDissolveProgress, noise);
		#else
			float noise = cnoise(uv/uInvTexResolution0 * uDissolveZoom);
			float value = aaStep((uDissolveProgress-0.5)*2.0, vec4(noise));
		#endif

		#ifdef RENDER_NOISE
			finalColor = vec4(noise);
		#else
			finalColor.a = value;
		#endif
	#endif

	#ifdef KILL_ALPHA
		if (finalColor.a <= 0.001)
			discard;
	#endif

	fragColor = finalColor;
#endif
}