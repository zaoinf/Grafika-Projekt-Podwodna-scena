#version 430 core

float AMBIENT = 0.3;

uniform vec3 cameraPos;

uniform vec3 color;

uniform vec3 lightPos;
uniform vec3 lightColor;

uniform sampler2D colorTexture; // The texture to sample

uniform vec3 spotlightPos;
uniform vec3 spotlightColor;
uniform vec3 spotlightConeDir;
uniform vec3 spotlightPhi;

uniform float exposition;

in vec3 vecNormal;
in vec3 worldPos;

in vec2 TexCoord; // Received from vertex shader

in vec3 viewDirTS;
in vec3 lightDirTS;
in vec3 spotlightDirTS;
in vec3 test;

out vec4 outColor;
out vec4 FragColor;

vec3 phongLight(vec3 lightDir, vec3 lightColor, vec3 normal, vec3 viewDir) 
{
    float diffuse = max(0, dot(normal, lightDir));

    vec3 R = reflect(-lightDir, normal);  
    float specular = pow(max(dot(viewDir, R), 0.0), 32);

    vec3 resultColor = color * diffuse * lightColor + lightColor * specular;
    return resultColor;
}

void main()
{
    vec3 normal = normalize(vecNormal);
    vec3 viewDir = normalize(viewDirTS);
    vec3 lightDir = normalize(lightPos - worldPos);

    vec3 ambient = AMBIENT * color;
    vec3 attenuatedlightColor = lightColor / pow(length(lightPos - worldPos), 2);
    vec3 illumination;
    
    // Base illumination with direct light (Phong lighting)
    illumination = ambient + phongLight(normalize(lightDirTS), attenuatedlightColor, normal, viewDir);
    
    // Spotlight
    vec3 spotlightDir = normalize(spotlightPos - worldPos);
    float angle_atenuation = clamp((dot(-spotlightDir, spotlightConeDir) - 0.8) * 3, 0, 1);
    
    attenuatedlightColor = spotlightColor / pow(length(spotlightPos - worldPos), 2) * angle_atenuation;
    illumination = illumination + phongLight(normalize(spotlightDirTS), attenuatedlightColor, normal, viewDir);

    // Apply exposition to simulate lighting response
    outColor = vec4(1.0 - exp(-illumination * exposition), 1.0);
        // Sample the texture and combine with illumination
    vec4 texColor = texture(colorTexture, TexCoord);
    FragColor = texColor * outColor;  // Final color: texture color modulated by lighting

 
}
