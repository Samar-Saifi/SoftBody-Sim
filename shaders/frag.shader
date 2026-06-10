#version 330 core

out vec4 FragColor;

in vec3 fragPos;
in vec3 normal;
in vec4 fragPosLight;

uniform vec4 color;
uniform vec3 lightPos;
uniform vec3 lightColor;
uniform vec3 viewPos;
uniform sampler2D shadowMap;

float CalculateShadows(vec4 fragPosL, vec3 norm, vec3 lightDir){
    vec3 projCoords = fragPosL.xyz / fragPosL.w;

    projCoords = projCoords * 0.5 + 0.5;

    if(projCoords.z > 1.0)
        return 0.0;

    float closestDepth = texture(shadowMap, projCoords.xy).r;
    float currentDepth = projCoords.z;
    float bias = max(0.05 * (1.0 - dot(norm, lightDir)), 0.005);
    float shadow = (currentDepth - bias) > closestDepth  ? 1.0 : 0.0;

    return shadow;
}

void main()
{
    float ambientStrength = 0.2;

    vec3 norm = normalize(normal);
    vec3 ambient = ambientStrength * lightColor;
    vec3 lightDir = normalize(lightPos - fragPos);
    float diffRate = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diffRate * lightColor;
    float shadow = CalculateShadows(fragPosLight, norm, lightDir);
    vec3 result = (ambient + (1.0 - shadow) * diffuse) * vec3(color);
    FragColor = vec4(result, color.a);
}
