#version 330 core

layout(location = 0) in vec3 aPos;

uniform mat4 transformation;

// Implementacja 2D szumu Perlina
vec2 random2(vec2 st) {
    return fract(
        sin(
            vec2(
                dot(st, vec2(127.1, 311.7)),
                dot(st, vec2(269.5, 183.3))
            )
        ) * 43758.5453
    );
}

float perlinNoise(vec2 st) {
    vec2 i = floor(st);
    vec2 f = fract(st);

    // Losowe gradienty
    vec2 grad00 = normalize(random2(i + vec2(0.0, 0.0)) * 2.0 - 1.0);
    vec2 grad10 = normalize(random2(i + vec2(1.0, 0.0)) * 2.0 - 1.0);
    vec2 grad01 = normalize(random2(i + vec2(0.0, 1.0)) * 2.0 - 1.0);
    vec2 grad11 = normalize(random2(i + vec2(1.0, 1.0)) * 2.0 - 1.0);

    // Wektory odleg³oœci
    vec2 d00 = f - vec2(0.0, 0.0);
    vec2 d10 = f - vec2(1.0, 0.0);
    vec2 d01 = f - vec2(0.0, 1.0);
    vec2 d11 = f - vec2(1.0, 1.0);

    // Iloczyny skalarne
    float dot00 = dot(grad00, d00);
    float dot10 = dot(grad10, d10);
    float dot01 = dot(grad01, d01);
    float dot11 = dot(grad11, d11);

    // Interpolacja
    vec2 u = smoothstep(0.0, 1.0, f);
    float lerpX0 = mix(dot00, dot10, u.x);
    float lerpX1 = mix(dot01, dot11, u.x);
    float value = mix(lerpX0, lerpX1, u.y);

    // Normalizacja do [0, 1]
    return value * 0.5 + 0.5;
}

void main() {
    // Generuj wysokoœæ za pomoc¹ szumu Perlina 2D
    vec2 noiseInput = aPos.xz * 0.5; // Skalowanie wspó³rzêdnych
    float noise = perlinNoise(noiseInput) * 4.0; // Wzmocnienie wysokoœci
    
    vec3 modifiedPos = vec3(aPos.x, noise, aPos.z);
    gl_Position = transformation * vec4(modifiedPos, 1.0);
} 