#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

out VS_OUT {
    vec2 texCoords;
	vec3 normal;
	vec3 worldPos;
	vec4 posInLightSpace;
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
uniform mat4 uLightSpaceMatrix;
uniform float uTexTiling = 1.0f;
uniform vec2 uTexDisplacement = vec2(0.0);

void main()
{
	vs_out.worldPos = vec3(uModel * vec4(aPos, 1.0));

	vs_out.normal = transpose(inverse(mat3(uModel))) * aNormal;

	vs_out.texCoords = aTexCoords * uTexTiling + uTexDisplacement;

	vs_out.posInLightSpace = uLightSpaceMatrix * vec4(vs_out.worldPos, 1.0);

	gl_Position = uProjection * uView * vec4(vs_out.worldPos, 1.0);
}
