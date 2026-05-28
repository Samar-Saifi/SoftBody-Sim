#version 330 core

out vec4 FragColor;

in vec3 fragPos;
in vec3 normal;

uniform vec4 color;
uniform vec3 lightPos;
uniform vec3 lightColor;

void main()
{
    float ambientStrength = 0.2;

    vec3 norm = normalize(normal);
    vec3 ambient = ambientStrength * lightColor;
    vec3 lightDir = normalize(lightPos - fragPos);
    float diffRate = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diffRate * lightColor;
    vec3 result = (ambient + diffuse) * vec3(color);
    FragColor = vec4(result, color.a);
}
