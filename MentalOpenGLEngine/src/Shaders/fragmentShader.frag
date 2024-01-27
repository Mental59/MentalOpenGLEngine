#version 330 core

out vec4 FragColor;

in vec2 texCoords;

uniform sampler2D uTexture1;
uniform sampler2D uTexture2;

void main()
{
	FragColor = mix(
		texture(uTexture1, texCoords),
		texture(uTexture2, texCoords),
		0.2
	);
}
