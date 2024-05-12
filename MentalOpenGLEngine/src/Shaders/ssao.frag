#version 330 core

#define MAX_SAMPLES 256

in vec2 vTexCoords;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D uNoiseTexture;

uniform vec3 uSamples[MAX_SAMPLES];
uniform int uNumSamples;
uniform vec2 uNoiseScale;

layout (location = 0) out float Occlusion;

void main()
{
	vec3 worldPos = texture(gPosition, vTexCoords).rgb;
	vec3 normal = texture(gNormal, vTexCoords).rgb;
	vec3 noise = texture(uNoiseTexture, vTexCoords * uNoiseScale).rgb;

	vec3 tangent = normalize(noise - normal * dot(noise, normal));
	vec3 bitangent = cross(normal, tangent);
	mat3 TBN = mat3(tangent, bitangent, normal);

	float occlusion = 0.0;
	for (int i = 0; i < uNumSamples; i++)
	{
		vec3 samplePos = TBN * uSamples[i];

	}

	Occlusion = 0.0;
}
