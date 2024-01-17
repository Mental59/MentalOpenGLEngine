#version 330 core

out vec4 FragColor;

in vec3 vertexColor;
in vec2 texCoords;

uniform sampler2D uTexture1;

void main()
{
	FragColor = texture(uTexture1, texCoords) * vec4(vertexColor, 1.0);
}
