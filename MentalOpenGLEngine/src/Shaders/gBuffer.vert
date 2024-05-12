#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in vec3 aTangent;

out VS_OUT {
    vec2 texCoords;
	vec3 normal;
	vec3 worldPos;
	vec3 posInViewSpace;
	mat3 tangentToWorld;
	float normalsMultiplier;

	vec3 tangentPos;
	vec3 tangentViewPos;
} vs_out;

layout (std140) uniform Matrices
{
	mat4 uView;
	mat4 uProjection;
};

uniform mat4 uModel;
uniform float uTexTiling = 1.0f;
uniform vec2 uTexDisplacement = vec2(0.0);
uniform float uNormalsMultiplier = 1.0;
uniform vec3 uViewPos;

mat3 TBNMat(const vec3 normal, const mat3 normalMatrix);

void main()
{
	vec3 normal = aNormal * uNormalsMultiplier;

	vs_out.worldPos = vec3(uModel * vec4(aPos, 1.0));

	mat3 worldNormalMatrix = transpose(inverse(mat3(uModel)));
	vs_out.normal = normalize(worldNormalMatrix * normal);

	vs_out.texCoords = aTexCoords * uTexTiling + uTexDisplacement;

	vs_out.tangentToWorld = TBNMat(aNormal, worldNormalMatrix);

	vs_out.normalsMultiplier = uNormalsMultiplier;

	mat3 worldToTangent = transpose(vs_out.tangentToWorld);
	vs_out.tangentPos = worldToTangent * vs_out.worldPos;
	vs_out.tangentViewPos = worldToTangent * uViewPos;

	gl_Position = uProjection * uView * vec4(vs_out.worldPos, 1.0);
}

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
