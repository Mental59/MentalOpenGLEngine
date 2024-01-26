#version 330 core

out vec4 FragColor;

in vec3 vertexColor;
in vec2 texCoords;

uniform sampler2D uTexture1;
uniform sampler2D uTexture2;
uniform float uMixAlpha;

void main()
{
	FragColor = mix(
		texture(uTexture1, texCoords),
		texture(uTexture2, vec2(1 - texCoords.x, texCoords.y)),
		uMixAlpha
	);
}
