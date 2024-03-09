#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;

uniform mat4 uView;
uniform mat4 uModel;

out VS_OUT {
	vec3 normal;
} vs_out;

void main()
{
	mat3 normalMatrix = mat3(transpose(inverse(uView * uModel)));
	vs_out.normal = normalMatrix * aNormal;
	gl_Position = uView * uModel * vec4(aPos, 1.0); 
}
