#version 430 core

uniform float time;  // czas dla perlin noise
uniform vec3 cameraPos;  // pozycja kamery
uniform vec3 fogColor = vec3(0.2, 0.5, 0.7);  // kolor mg³y
uniform float fogDensity = 0.05;  // gêstoœæ mg³y tu regulujesz
uniform sampler2D colorTexture;  // tekstura

in vec3 vecNormal;
in vec3 worldPos;
in vec2 vecTex;

out vec4 outColor;

// Perlin noise generuje "randomow¹" liczbe miêdzy 0.0-1.0 
float hash(vec2 p) 
{
    return fract(sin(dot(p, vec2(127.1, 311.7))) * 43758.5453123);
}

// Perlin noise 
float noise(vec2 p) 
{
    vec2 i = floor(p);
    vec2 f = fract(p);
    vec2 u = f * f * (3.0 - 2.0 * f);
    return mix(mix(hash(i + vec2(0.0, 0.0)), hash(i + vec2(1.0, 0.0)), u.x),mix(hash(i + vec2(0.0, 1.0)), hash(i + vec2(1.0, 1.0)), u.x),u.y);
}

// oblicza zniekszta³cenie (distortion) a konkretnie to koordynaty
vec2 calculateDistortion(vec2 texCoord, float time) 
{
    vec2 noiseCoord = texCoord * 5.0 + vec2(time * 0.1, time * 0.15);
    float distortion = noise(noiseCoord) * 0.3;
    return texCoord + vec2(distortion, distortion);
}


// Robi mg³e
vec3 applyFog(vec3 color, vec3 fogColor, float distance, float fogDensity) 
{
    float fogFactor = exp(-fogDensity * distance);
    return mix(fogColor, color, fogFactor);
}


void main() 
{
    //  koordynaty zniekszta³cenia (distortion)
    vec2 distortedTex = calculateDistortion(vecTex, time);

    // pobiera texture i dodaje distortion 
    vec3 textureColor = texture(colorTexture, distortedTex).rgb;
    vec3 tintedColor = mix(textureColor, vec3(0.1, 0.4, 0.5), 0.3);

    // Dodaje mg³e 
    float distance = length(cameraPos - worldPos);
    vec3 finalColor = applyFog(tintedColor, fogColor, distance, fogDensity);

    // Wysy³a kolor do renderowania
    outColor = vec4(finalColor, 1.0);
}
