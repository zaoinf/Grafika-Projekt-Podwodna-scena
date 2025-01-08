#version 430 core

uniform float time; // Time uniform to animate noise
uniform vec3 cameraPos;
uniform vec3 fogColor = vec3(0.2, 0.5, 0.7); // Underwater fog color
uniform float fogDensity = 0.05; // Fog density
uniform sampler2D colorTexture;

// Perlin noise function
float hash(vec2 p) {
    return fract(sin(dot(p, vec2(127.1, 311.7))) * 43758.5453123);
}

float noise(vec2 p) {
    vec2 i = floor(p);
    vec2 f = fract(p);
    vec2 u = f * f * (3.0 - 2.0 * f);
    return mix(mix(hash(i + vec2(0.0, 0.0)), hash(i + vec2(1.0, 0.0)), u.x),
               mix(hash(i + vec2(0.0, 1.0)), hash(i + vec2(1.0, 1.0)), u.x), u.y);
}

in vec3 vecNormal;
in vec3 worldPos;
in vec2 vecTex;

out vec4 outColor;

void main()
{
    // Animate Perlin noise for distortion
    vec2 noiseCoord = vecTex * 5.0 + vec2(time * 0.1, time * 0.15);
    float distortion = noise(noiseCoord) * 0.1;

    // Apply distortion to texture coordinates
    vec2 distortedTex = vecTex + vec2(distortion, distortion);

    // Sample the texture with distorted coordinates
    vec3 textureColor = texture(colorTexture, distortedTex).rgb;

    // Add a blue-green tint to simulate underwater light scattering
    vec3 tintedColor = mix(textureColor, vec3(0.1, 0.4, 0.5), 0.3);

    // Calculate fog based on distance
    float distance = length(cameraPos - worldPos);
    float fogFactor = exp(-fogDensity * distance);

    // Blend the fog color with the scene
    vec3 finalColor = mix(fogColor, tintedColor, fogFactor);

    outColor = vec4(finalColor, 1.0);
}
