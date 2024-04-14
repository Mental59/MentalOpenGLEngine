#version 330 core

out vec4 FragColor;

in VS_OUT {
    vec2 texCoords;
	vec3 normal;
	vec3 worldPos;
	vec4 posInLightSpace;
	mat3 TBN;
	float normalsMultiplier;
} fs_in;

uniform vec3 uViewPos;
uniform samplerCube uSkybox;

void main()
{             
    vec3 I = normalize(fs_in.worldPos - uViewPos);
//    vec3 R = refract(I, normalize(fs_in.normal), 1.0 / 1.52);
    vec3 R = reflect(I, normalize(fs_in.normal));
    FragColor = vec4(texture(uSkybox, R).rgb, 1.0);
}
