#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;
layout (location = 2) in vec2 aTexCoords;

out vec3 vertexColor;
out vec2 texCoords;

uniform mat4 uTransform;

void main()
{
	gl_Position = uTransform * vec4(aPos, 1.0);
	vertexColor = aColor;
	texCoords = aTexCoords;
}
