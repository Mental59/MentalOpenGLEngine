#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in vec3 aTangent;

out VS_OUT {
    vec2 texCoords;
	vec3 normal;
	vec3 worldPos;
	vec4 posInLightSpace;
	mat3 tangentToWorld;
	float normalsMultiplier;

	vec3 tangentPos;
	vec3 tangentViewPos;
	vec3 worldViewPos;
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
uniform float uNormalsMultiplier = 1.0;
uniform vec3 uViewPos;

mat3 TBNMat(const vec3 normal, const mat3 normalMatrix)
{
	vec3 T = normalize(normalMatrix * aTangent);
	vec3 N = normalize(normalMatrix * normal);

	// re-orthogonalize T with respect to N
	T = normalize(T - dot(T, N) * N);

	// then retrieve perpendicular vector B with the cross product of T and N
	vec3 B = cross(T, N);

	mat3 TBN = mat3(T, B, N);

	return TBN;
}

void main()
{
	vec3 normal = aNormal * uNormalsMultiplier;

	vs_out.worldPos = vec3(uModel * vec4(aPos, 1.0));

	mat3 normalMatrix = transpose(inverse(mat3(uModel)));
	vs_out.normal = normalize(normalMatrix * normal);

	vs_out.texCoords = aTexCoords * uTexTiling + uTexDisplacement;

	vs_out.posInLightSpace = uLightSpaceMatrix * vec4(vs_out.worldPos, 1.0);

	vs_out.tangentToWorld = TBNMat(aNormal, normalMatrix);

	vs_out.normalsMultiplier = uNormalsMultiplier;

	vs_out.tangentPos = transpose(vs_out.tangentToWorld) * vs_out.worldPos;
	vs_out.tangentViewPos = transpose(vs_out.tangentToWorld) * uViewPos;
	vs_out.worldViewPos = uViewPos;

	gl_Position = uProjection * uView * vec4(vs_out.worldPos, 1.0);
}
