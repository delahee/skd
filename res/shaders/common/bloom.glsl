#ifndef _HBC_GL_BLOOM
#define _HBC_GL_BLOOM

#define EPSILON         1.0e-4

//
// Quadratic color thresholding
// curve = (threshold - knee, knee * 2, 0.25 / knee)
//
vec4 QuadraticThreshold(vec4 color, float threshold, vec3 curve)
{
    // Pixel brightness
    float br = max(max(color.r, color.g), color.b);

    // Under-threshold part: quadratic curve
    float rq = clamp(br - curve.x, 0.0, curve.y);
    rq = curve.z * rq * rq;

    // Combine and apply the brightness response curve.
    color *= max(rq, br - threshold) / max(br, EPSILON);

    return color;
}

// . . . . . . .
// . A . B . C .
// . . D . E . .
// . F . G . H .
// . . I . J . .
// . K . L . M .
// . . . . . . .
vec4 DownsampleBox13Tap(vec2 uv, vec2 texelSize) {
    vec4 A = texture2D(uTexture0, uv + texelSize * vec2(-1.0, -1.0));
    vec4 B = texture2D(uTexture0, uv + texelSize * vec2( 0.0, -1.0));
    vec4 C = texture2D(uTexture0, uv + texelSize * vec2( 1.0, -1.0));
    vec4 D = texture2D(uTexture0, uv + texelSize * vec2(-0.5, -0.5));
    vec4 E = texture2D(uTexture0, uv + texelSize * vec2( 0.5, -0.5));
    vec4 F = texture2D(uTexture0, uv + texelSize * vec2(-1.0,  0.0));
    vec4 G = texture2D(uTexture0, uv                               );
    vec4 H = texture2D(uTexture0, uv + texelSize * vec2( 1.0,  0.0));
    vec4 I = texture2D(uTexture0, uv + texelSize * vec2(-0.5,  0.5));
    vec4 J = texture2D(uTexture0, uv + texelSize * vec2( 0.5,  0.5));
    vec4 K = texture2D(uTexture0, uv + texelSize * vec2(-1.0,  1.0));
    vec4 L = texture2D(uTexture0, uv + texelSize * vec2( 0.0,  1.0));
    vec4 M = texture2D(uTexture0, uv + texelSize * vec2( 1.0,  1.0));

    vec2 div = (1.0 / 4.0) * vec2(0.5, 0.125);

    vec4 o  = (D + E + I + J) * div.x;
         o += (A + B + G + F) * div.y;
         o += (B + C + H + G) * div.y;
         o += (F + G + L + K) * div.y;
         o += (G + H + M + L) * div.y;

    return o;
}

// 9-tap bilinear upsampler (tent filter)
vec4 UpsampleTent(sampler2D tex, vec2 uv, vec2 texelSize) {
    float sampleScale = 1.0;
    vec4 d = texelSize.xyxy * vec4(1.0, 1.0, -1.0, 0.0) * sampleScale;

    vec4 s;
    s  = texture2D(tex, uv - d.xy);
    s += texture2D(tex, uv - d.wy) * 2.0;
    s += texture2D(tex, uv - d.zy);
    
    s += texture2D(tex, uv + d.zw) * 2.0;
    s += texture2D(tex, uv       ) * 4.0;
    s += texture2D(tex, uv + d.xw) * 2.0;
    
    s += texture2D(tex, uv + d.zy);
    s += texture2D(tex, uv + d.wy) * 2.0;
    s += texture2D(tex, uv + d.xy);

    return s * (1.0 / 16.0);
}

#endif // _HBC_GL_BLOOM