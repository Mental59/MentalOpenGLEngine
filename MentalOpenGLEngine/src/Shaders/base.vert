#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

out vec2 vTexCoords;
out vec3 vNormal;
out vec3 vFragmentPos;
out vec3 vLightPos;

uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProjection;

uniform vec3 uLightWorldPos;

void main()
{
	vFragmentPos = vec3(uView * uModel * vec4(aPos, 1.0));

	vNormal = transpose(inverse(mat3(uView * uModel))) * aNormal;

	vLightPos = vec3(uView * vec4(uLightWorldPos, 1.0));

	vTexCoords = aTexCoords;

	gl_Position = uProjection * vec4(vFragmentPos, 1.0);
}
