#version 330 core

out vec4 FragColor;

uniform vec3 uOutlineColor = vec3(1.0, 0.0, 0.0);

void main()
{
    FragColor = vec4(uOutlineColor, 1.0);
}
