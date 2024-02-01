#version 330 core

out vec4 FragColor;

in vec2 vTexCoords;
in vec3 vLightColor;

uniform vec3 uObjectColor;

void main()
{
	FragColor = vec4(vLightColor * uObjectColor, 1.0f);
}
