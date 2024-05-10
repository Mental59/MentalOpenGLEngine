#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 4) in mat4 aInstanceModelMatrix;

uniform mat4 uLightSpaceMatrix;

void main()
{
	gl_Position = uLightSpaceMatrix * aInstanceModelMatrix * vec4(aPos, 1.0);
}
