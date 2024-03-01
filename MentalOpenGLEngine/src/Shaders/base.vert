#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

out vec2 vTexCoords;
out vec3 vNormal;
out vec3 vWorldPos;

layout (std140) uniform Matrices
{
	mat4 uView;
	mat4 uProjection;
};

//layout (std140) uniform Test
//{
//	float uField1;
//	vec3 uField2;
//	mat4 uField3;
//	float uField4[3];
//	bool uField5;
//	int uField6;
//};

uniform mat4 uModel;
uniform float texCoordsMultiplier = 1.0;

void main()
{
	vWorldPos = vec3(uModel * vec4(aPos, 1.0));

	vNormal = transpose(inverse(mat3(uModel))) * aNormal;

	vTexCoords = aTexCoords * texCoordsMultiplier;

	gl_Position = uProjection * uView * vec4(vWorldPos, 1.0);
}
