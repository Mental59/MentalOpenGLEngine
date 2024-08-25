#version 330 core

out vec4 FragColor;

in vec3 vTexCoords;

uniform samplerCube uSkybox;

void main()
{
    vec3 envColor = textureLod(uSkybox, vTexCoords, 0).rgb;
    FragColor = vec4(envColor, 1.0);
}
