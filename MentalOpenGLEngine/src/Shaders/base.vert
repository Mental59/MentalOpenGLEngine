#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

out VS_OUT {
    vec2 texCoords;
	vec3 normal;
	vec3 worldPos;
} vs_out;

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
uniform float uTexCoordsMultiplier = 1.0;

void main()
{
	vs_out.worldPos = vec3(uModel * vec4(aPos, 1.0));

	vs_out.normal = transpose(inverse(mat3(uModel))) * aNormal;

	vs_out.texCoords = aTexCoords * uTexCoordsMultiplier;

	gl_Position = uProjection * uView * vec4(vs_out.worldPos, 1.0);
}
