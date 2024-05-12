#version 330 core

#define MAX_SAMPLES 256

in vec2 vTexCoords;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D uNoiseTexture;

uniform vec3 uSamples[MAX_SAMPLES];
uniform int uNumSamples;
uniform vec2 uNoiseScale;
uniform float uRadius = 0.5;
uniform float uBias = 0.025;
uniform float uPower = 1.0;

layout (std140) uniform Matrices
{
	mat4 uView;
	mat4 uProjection;
};

layout (location = 0) out float Occlusion;

void main()
{
	vec3 fragPos = texture(gPosition, vTexCoords).rgb;
	vec3 normal = normalize(texture(gNormal, vTexCoords).rgb);
	vec3 noise = normalize(texture(uNoiseTexture, vTexCoords * uNoiseScale).rgb);

	// create TBN change-of-basis matrix: from tangent-space to view-space
	vec3 tangent = normalize(noise - normal * dot(noise, normal));
	vec3 bitangent = cross(normal, tangent);
	mat3 TBN = mat3(tangent, bitangent, normal);

	float occlusion = 0.0;
	for (int i = 0; i < min(uNumSamples, MAX_SAMPLES); i++)
	{
		// get sample position
		vec3 samplePos = TBN * uSamples[i]; // from tangent to view space
		samplePos = fragPos + samplePos * uRadius;

		// project sample position (to sample texture) (to get position on screen/texture)
		vec4 offset = vec4(samplePos, 1.0);
		offset = uProjection * offset; // from view to clip-space
		offset.xyz /= offset.w; // perspective divide
		offset.xyz = offset.xyz * 0.5 + 0.5; // transform to range 0.0 - 1.0 

		// get sample depth
		float sampleDepth = texture(gPosition, offset.xy).z; // get depth value of kernel sample

		// range check & accumulate
		float rangeCheck = smoothstep(0.0, 1.0, uRadius / abs(fragPos.z - sampleDepth));
		occlusion += (sampleDepth >= samplePos.z + uBias ? 1.0 : 0.0) * rangeCheck;
	}
	occlusion = 1.0 - (occlusion / uNumSamples);
	occlusion = pow(occlusion, uPower);

	Occlusion = occlusion;
}
