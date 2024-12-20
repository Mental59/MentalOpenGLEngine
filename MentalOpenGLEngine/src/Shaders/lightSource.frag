#version 330 core

layout (location = 0) out vec4 FragColor;

uniform vec3 uLightColor = vec3(1.0, 1.0, 1.0);

void main()
{
    FragColor = vec4(uLightColor, 1.0);
}
