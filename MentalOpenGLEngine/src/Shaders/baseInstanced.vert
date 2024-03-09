#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in mat4 instanceMatrix;

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

void main()
{
	vs_out.worldPos = vec3(instanceMatrix * vec4(aPos, 1.0));

	vs_out.normal = transpose(inverse(mat3(instanceMatrix))) * aNormal;

	vs_out.texCoords = aTexCoords;

	gl_Position = uProjection * uView * vec4(vs_out.worldPos, 1.0);
}
