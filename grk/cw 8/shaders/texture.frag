#version 430 core

float AMBIENT = 0.1;

uniform vec3 color;
uniform vec3 lightPos = vec3(0.0, 100.0, 0.0);
uniform sampler2D colorTexture;

in vec3 vecNormal;
in vec3 worldPos;
in vec2 vecTex;

out vec4 outColor;

void main()
{
    vec3 textureColor = texture2D(colorTexture, vecTex).xyz;
    outColor = vec4(textureColor, 1.0); // Directly output the texture color
}