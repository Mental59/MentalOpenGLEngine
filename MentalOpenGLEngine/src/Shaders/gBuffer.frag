#version 330 core

in VS_OUT {
    vec2 texCoords;
	vec3 normal;
	vec3 worldPos;
	vec3 posInViewSpace;
	mat3 tangentToWorld;
	float normalsMultiplier;

	vec3 tangentPos;
	vec3 tangentViewPos;
} fs_in;

struct Material
{
	sampler2D diffuseTexture1;
	sampler2D specularTexture1;
	sampler2D normalTexture1;
	sampler2D heightTexture1;

	vec3 specular;

	bool useNormalTexture;
	bool useSpecularTexture;
	bool useHeightTexture;
};

layout (location = 0) out vec3 gPosition;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec4 gAlbedoSpecular;

uniform Material uMaterial;
uniform float uHeightScale = 0.1;

vec2 ParallaxOcclusionMapping(const vec2 texCoords, const vec3 viewDirection, const float minLayers, const float maxLayers);

void main()
{
	vec3 viewDirectionTangent = normalize(fs_in.tangentViewPos - fs_in.tangentPos);

	vec2 texCoords = uMaterial.useHeightTexture ? ParallaxOcclusionMapping(fs_in.texCoords, viewDirectionTangent, 8.0, 32.0) : fs_in.texCoords;

	vec3 normal = normalize(fs_in.normal);
	if (uMaterial.useNormalTexture)
	{
		normal = texture(uMaterial.normalTexture1, texCoords).rgb;
		normal = normalize(normal * 2.0 - 1.0); // transform to range [-1, 1]
		normal = fs_in.tangentToWorld * normal;
		normal *= fs_in.normalsMultiplier;
	}

	vec3 albedo = texture(uMaterial.diffuseTexture1, texCoords).rgb;
	albedo = vec3(0.95);

	float specular = uMaterial.specular.r;
	if (uMaterial.useSpecularTexture)
	{
		specular = texture(uMaterial.specularTexture1, texCoords).r;
	}

	gPosition = fs_in.worldPos;
	gNormal = normal;
	gAlbedoSpecular = vec4(albedo, specular);
}

vec2 ParallaxOcclusionMapping(const vec2 texCoords, const vec3 viewDirection, const float minLayers, const float maxLayers)
{
	// number of depth layers
	float numLayers = mix(maxLayers, minLayers, abs(dot(vec3(0.0, 0.0, 1.0), viewDirection)));

	// calculate the size of each layer
	float layerDepth = 1.0 / numLayers;
	// depth of current layer
	float currentLayerDepth = 0.0;

	// the amount to shift the texture coordinates per layer (from vector P)
	vec2 p = viewDirection.xy * uHeightScale;
	vec2 deltaTexCoords = p / numLayers;

	// set initial values
	vec2 currentTexCoords = texCoords;
	float currentDepthMapValue = texture(uMaterial.heightTexture1, texCoords).r;
	
	while(currentLayerDepth < currentDepthMapValue)
	{
		// shift texture coordinates along direction of P
		currentTexCoords -= deltaTexCoords;
		// get depthmap value at current texture coordinates
		currentDepthMapValue = texture(uMaterial.heightTexture1, currentTexCoords).r;  
		// get depth of next layer
		currentLayerDepth += layerDepth;  
	}

	// get texture coordinates before collision (reverse operations)
	vec2 prevTexCoords = currentTexCoords + deltaTexCoords;

	// get depth after and before collision for linear interpolation
	float afterDepth  = currentDepthMapValue - currentLayerDepth;
	float beforeDepth = texture(uMaterial.heightTexture1, prevTexCoords).r - (currentLayerDepth - layerDepth);

	// interpolation of texture coordinates
	float weight = afterDepth / (afterDepth - beforeDepth);
	vec2 finalTexCoords = prevTexCoords * weight + currentTexCoords * (1.0 - weight);

	return finalTexCoords;
}
