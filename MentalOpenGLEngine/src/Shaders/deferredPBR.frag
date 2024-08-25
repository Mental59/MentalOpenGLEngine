#version 330 core

struct PointLight
{
	vec3 position;
	vec3 color;

	float farPlane;
	samplerCube shadowCubeMap;
};

float CalculatePointShadow(PointLight light, const vec3 fragPos, const vec3 viewPos);

#define MAX_POINT_LIGHTS 16
const float PI = 3.14159265359;

in vec2 vTexCoords;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedoSpec;
uniform sampler2D gMetallicRoughnessAO;

uniform sampler2D uSSAOTexture;
uniform samplerCube uIrradianceMap;
uniform samplerCube uPrefilterMap;
uniform sampler2D uBrdfLutMap;

uniform vec3 uViewPos;

uniform PointLight uPointLights[MAX_POINT_LIGHTS];
uniform int uNumPointLights = 0;

layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 BrightColor;

float DistributionGGX(const vec3 N, const vec3 H, const float roughness);
float GeometrySchlickGGX(float NdotV, float roughness);
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness);
vec3 FresnelSchlick(float cosTheta, vec3 F0);
vec3 FresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness);

const float MAX_REFLECTION_LOD = 4.0;

void main()
{
	vec3 worldPos = texture(gPosition, vTexCoords).rgb;
	vec3 normal = texture(gNormal, vTexCoords).rgb;
	vec4 albedoSpecular = texture(gAlbedoSpec, vTexCoords);
	vec4 metallicRoughnessAO = texture(gMetallicRoughnessAO, vTexCoords);
	float ambientOcclusion = texture(uSSAOTexture, vTexCoords).r;

	vec3 albedo = albedoSpecular.rgb;

	float metallic = metallicRoughnessAO.r;
	float roughness = metallicRoughnessAO.g;
	float ao = metallicRoughnessAO.b;

	vec3 viewDirection = normalize(uViewPos - worldPos);

	vec3 F0 = vec3(0.04);
	F0 = mix(F0, albedo, metallic);

	vec3 Lo = vec3(0.0);
	for (int i = 0; i < min(MAX_POINT_LIGHTS, uNumPointLights); i++)
	{
		float dist = length(uPointLights[i].position - worldPos);
		float attenuation = 1.0 / (dist * dist);
		vec3 radiance = uPointLights[i].color * attenuation;

		vec3 lightDirection = normalize(uPointLights[i].position - worldPos);
		vec3 halfVector = normalize(viewDirection + lightDirection);

		float NDF = DistributionGGX(normal, halfVector, roughness);
		float G = GeometrySmith(normal, viewDirection, lightDirection, roughness);
		vec3 F = FresnelSchlick(clamp(dot(halfVector, viewDirection), 0.0, 1.0), F0);

		float NdotL = max(dot(normal, lightDirection), 0.0);
		float NdotV = max(dot(normal, viewDirection), 0.0);

		vec3 specular = (NDF * G * F) / (4.0 * NdotV * NdotL + 0.0001);
		
		vec3 kS = F;
		vec3 kD = (vec3(1.0) - kS) * (1.0 - metallic);

		Lo += (kD * albedo / PI + specular) * radiance * NdotL;
	}

	vec3 F = FresnelSchlickRoughness(max(dot(normal, viewDirection), 0.0), F0, roughness);
	vec3 kS = F;
	vec3 kD = 1.0 - kS;
	kD *= 1.0 - metallic;

	vec3 irradiance = texture(uIrradianceMap, normal).rgb;
	vec3 diffuse = irradiance * albedo;

	vec3 R = reflect(-viewDirection, normal);
	vec3 prefilteredColor = textureLod(uPrefilterMap, R, roughness * MAX_REFLECTION_LOD).rgb;
	vec2 envBRDF = texture(uBrdfLutMap, vec2(max(dot(normal, viewDirection), 0.0), roughness)).rg;
	vec3 specular = prefilteredColor * (F * envBRDF.x + envBRDF.y);

	vec3 ambient = (kD * diffuse + specular) * ao;

	vec3 color = ambient + Lo;
	FragColor = vec4(color, 1.0);

	float brightness = dot(FragColor.rgb, vec3(0.2126, 0.7152, 0.0722));
    if (brightness > 1.0)
	{
		BrightColor = vec4(FragColor.rgb, 1.0);
	}  
    else
	{
		BrightColor = vec4(0.0, 0.0, 0.0, 1.0);
	}
}

float DistributionGGX(const vec3 N, const vec3 H, const float roughness)
{
	float a = roughness * roughness;
	float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float denom = (NdotH * NdotH * (a2 - 1.0) + 1.0);
    return a2 / (PI * denom * denom);
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
	float r = (roughness + 1.0);
    float k = (r*r) / 8.0;
    return NdotV / (NdotV * (1.0 - k) + k);
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    return GeometrySchlickGGX(NdotV, roughness) * GeometrySchlickGGX(NdotL, roughness);
}

vec3 FresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

vec3 FresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness)
{
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

float CalculatePointShadow(PointLight light, const vec3 fragPos, const vec3 viewPos)
{
	const int numSamples = 20;
	vec3 gridSamplingDisk[numSamples] = vec3[]
	(
	   vec3(1, 1,  1), vec3( 1, -1,  1), vec3(-1, -1,  1), vec3(-1, 1,  1), 
	   vec3(1, 1, -1), vec3( 1, -1, -1), vec3(-1, -1, -1), vec3(-1, 1, -1),
	   vec3(1, 1,  0), vec3( 1, -1,  0), vec3(-1, -1,  0), vec3(-1, 1,  0),
	   vec3(1, 0,  1), vec3(-1,  0,  1), vec3( 1,  0, -1), vec3(-1, 0, -1),
	   vec3(0, 1,  1), vec3( 0, -1,  1), vec3( 0, -1, -1), vec3( 0, 1, -1)
	);

	vec3 fragToLight = fragPos - light.position;
	float currentDepth = length(fragToLight);
	float shadow = 0.0;
	float bias   = 0.2;
	float viewDistance = length(viewPos - fragPos);
	float diskRadius = (1.0 + (viewDistance / light.farPlane)) / 50.0;

	for(int i = 0; i < numSamples; i++)
	{
		float closestDepth = texture(light.shadowCubeMap, fragToLight + gridSamplingDisk[i] * diskRadius).r;
		closestDepth *= light.farPlane;   // undo mapping [0;1]
		bool isInShadow = currentDepth - bias > closestDepth;
		shadow += float(isInShadow);
	}
	shadow /= float(numSamples);

    return shadow;
}
