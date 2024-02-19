#version 330 core

out vec4 FragColor;

in vec2 vTexCoords;

uniform sampler2D uScreenTexture;

void main()
{
    FragColor = vec4(texture(uScreenTexture, vTexCoords).rgb, 1.0);
}
