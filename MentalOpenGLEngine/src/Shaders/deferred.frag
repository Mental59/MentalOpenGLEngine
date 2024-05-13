#version 330 core

struct Material
{
	float shininess;
};
struct DirectionalLight
{
	vec3 direction;
  
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

	mat4 lightSpaceMat;
	sampler2D shadowMap;
};
struct PointLight
{
	vec3 position;
  
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

	float constant;
    float linear;
    float quadratic;

	float farPlane;
	float radius;

	samplerCube shadowCubeMap;
};
struct SpotLight
{
	vec3 position;
	vec3 direction;
	float cutOffCosine;
	float outerCutOffCosine;

	vec3 ambient;
	vec3 diffuse;
	vec3 specular;

	float constant;
    float linear;
    float quadratic;
};

vec3 CalculateDirectionalLight(
	DirectionalLight light,
	const vec3 normal,
	const vec3 viewDirection,
	const vec3 materialDiffuse,
	const vec3 materialSpecular,
	const float shadow,
	const float ambientOcclusion
);
vec3 CalculatePointLight(
	PointLight light,
	const vec3 normal,
	const vec3 viewDirection,
	const vec3 materialDiffuse,
	const vec3 materialSpecular,
	const vec3 fragPos,
	const float shadow,
	const float ambientOcclusion
);
vec3 CalculateSpotLight(
	SpotLight light,
	const vec3 normal,
	const vec3 viewDirection,
	const vec3 materialDiffuse,
	const vec3 materialSpecular,
	const vec3 fragPos,
	const float ambientOcclusion
);
vec3 CalculateAmbient(
	const vec3 lightAmbient,
	const vec3 materialDiffuse,
	const float ambientOcclusion
);
vec3 CalculateDiffuse(
	const vec3 lightDiffuse,
	const vec3 normal,
	const vec3 lightDirection,
	const vec3 materialDiffuse
);
vec3 CalculateSpecular(
	const vec3 lightSpecular,
	const vec3 normal,
	const vec3 lightDirection,
	const vec3 viewDirection,
	const vec3 materialSpecular
);
  
float LinearizeDepth(float depth, float near, float far);

float CalculateDirShadow(DirectionalLight light, const vec3 normal, const vec4 posInLightSpace);
float CalculatePointShadow(PointLight light, const vec3 fragPos, const vec3 viewPos);

#define MAX_POINT_LIGHTS 16
#define MAX_DIR_LIGHTS 1

in vec2 vTexCoords;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedoSpec;
uniform sampler2D uSSAOTexture;

uniform Material uMaterial;
uniform vec3 uViewPos;

uniform DirectionalLight uDirLights[MAX_DIR_LIGHTS];
uniform PointLight uPointLights[MAX_POINT_LIGHTS];
uniform int uNumPointLights = 0;
uniform int uNumDirLights = 0;

layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 BrightColor;

void main()
{
	vec3 worldPos = texture(gPosition, vTexCoords).rgb;
	vec3 normal = texture(gNormal, vTexCoords).rgb;
	vec4 albedoSpecular = texture(gAlbedoSpec, vTexCoords);
	vec3 albedo = albedoSpecular.rgb;
	float specular = albedoSpecular.a;
	float ambientOcclusion = texture(uSSAOTexture, vTexCoords).r;

	vec3 viewDirection = normalize(uViewPos - worldPos);

	vec3 color = vec3(0.0);
	float shadow = 0.0;
	for (int i = 0; i < min(MAX_DIR_LIGHTS, uNumDirLights); i++)
	{
		vec4 posInLightSpace = uDirLights[i].lightSpaceMat * vec4(worldPos, 1.0);
		shadow = CalculateDirShadow(uDirLights[i], normal, posInLightSpace);
		color += CalculateDirectionalLight(uDirLights[i], normal, viewDirection, albedo, vec3(specular), shadow, ambientOcclusion);
	}

	for (int i = 0; i < min(MAX_POINT_LIGHTS, uNumPointLights); i++)
	{
//		float distanceToLight = length(uPointLights[i].position - worldPos);
//		if (distanceToLight < uPointLights[i].radius)
//		{
		shadow = CalculatePointShadow(uPointLights[i], worldPos, uViewPos);
		color += CalculatePointLight(uPointLights[i], normal, viewDirection, albedo, vec3(specular), worldPos, shadow, ambientOcclusion);
//		}
	}
	
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

	// SSAO texture
//	FragColor = vec4(texture(uSSAOTexture, vTexCoords).rrr, 1.0);

	//dir light shadow map
//	FragColor = vec4(texture(uDirLights[0].shadowMap, vTexCoords).rrr, 1.0);
	
	// point light shadow cube map
//	vec3 fragToLight = worldPos - uPointLights[0].position;
//	float closestDepth = texture(uPointLights[0].shadowCubeMap, fragToLight).r;
//	FragColor = vec4(vec3(closestDepth), 1.0);
}

float CalculateDirShadow(DirectionalLight light, const vec3 normal, const vec4 posInLightSpace)
{
	// perform perspective division, so it works for perspective matrices too
    vec3 projCoords = posInLightSpace.xyz / posInLightSpace.w;

	// conversion to the range [0, 1]
	projCoords = projCoords * 0.5 + 0.5; 

	float currentDepth = projCoords.z;

	float bias = max(0.005 * (1.0 - dot(normal, light.direction)), 0.0005);

	// PCF
	float shadow = 0.0;
	vec2 texelSize = 1.0 / textureSize(light.shadowMap, 0);
	for (int x = -2; x <= 2; x++)
	{
		for (int y = -2; y <= 2; y++)
		{
			float pcfDepth = texture(light.shadowMap, projCoords.xy + vec2(x, y) * texelSize).r;
			bool isInShadow = currentDepth - bias > pcfDepth;
			shadow += float(isInShadow);
		}
	}
	shadow /= 25.0;

	return shadow;
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

vec3 CalculateDirectionalLight(
	DirectionalLight light,
	const vec3 normal,
	const vec3 viewDirection,
	const vec3 materialDiffuse,
	const vec3 materialSpecular,
	const float shadow,
	const float ambientOcclusion
)
{
	vec3 lightDirection = normalize(-light.direction);

	vec3 ambient = CalculateAmbient(light.ambient, materialDiffuse, ambientOcclusion);
	vec3 diffuse = CalculateDiffuse(light.diffuse, normal, lightDirection, materialDiffuse);
	vec3 specular = CalculateSpecular(light.specular, normal, lightDirection, viewDirection, materialSpecular);

	return ambient + (1.0 - shadow) * (diffuse + specular);
}

vec3 CalculatePointLight(
	PointLight light,
	const vec3 normal,
	const vec3 viewDirection,
	const vec3 materialDiffuse,
	const vec3 materialSpecular,
	const vec3 fragPos,
	const float shadow,
	const float ambientOcclusion
)
{
	vec3 lightDirection = normalize(light.position - fragPos);

	float distanceToLight = length(light.position - fragPos);
	float attenuation = 1.0 / (light.constant + light.linear * distanceToLight + light.quadratic * distanceToLight * distanceToLight);

	vec3 ambient = CalculateAmbient(light.ambient, materialDiffuse, ambientOcclusion);
	vec3 diffuse = CalculateDiffuse(light.diffuse, normal, lightDirection, materialDiffuse);
	vec3 specular = CalculateSpecular(light.specular, normal, lightDirection, viewDirection, materialSpecular);

	return attenuation * (ambient + (1.0 - shadow) * (diffuse + specular));
}

vec3 CalculateSpotLight(
	SpotLight light,
	const vec3 normal,
	const vec3 viewDirection,
	const vec3 materialDiffuse,
	const vec3 materialSpecular,
	const vec3 fragPos,
	const float ambientOcclusion
)
{
	vec3 lightDirection = normalize(-light.direction);
	vec3 lightDirectionFromFragment = normalize(light.position - fragPos);

	float distanceToLight = length(light.position - fragPos);
	float attenuation = 1.0 / (light.constant + light.linear * distanceToLight + light.quadratic * distanceToLight * distanceToLight);

	float theta = dot(lightDirectionFromFragment, normalize(-light.direction));
	float epsilon = light.cutOffCosine - light.outerCutOffCosine;
	float lightIntensity = clamp((theta - light.outerCutOffCosine) / epsilon, 0.0, 1.0);

	vec3 ambient = CalculateAmbient(light.ambient, materialDiffuse, ambientOcclusion);
	vec3 diffuse = CalculateDiffuse(light.diffuse, normal, lightDirection, materialDiffuse);
	vec3 specular = CalculateSpecular(light.specular, normal, lightDirection, viewDirection, materialSpecular);

	return lightIntensity * attenuation * (ambient + diffuse + specular); 
}

vec3 CalculateAmbient(
	const vec3 lightAmbient,
	const vec3 materialDiffuse,
	const float ambientOcclusion
)
{
	vec3 ambient = lightAmbient * materialDiffuse * ambientOcclusion;
	return ambient;
}

vec3 CalculateDiffuse(
	const vec3 lightDiffuse,
	const vec3 normal,
	const vec3 lightDirection,
	const vec3 materialDiffuse
)
{
	float diffuseFactor = max(dot(normal, lightDirection), 0.0);
	vec3 diffuse = diffuseFactor * lightDiffuse * materialDiffuse;
	return diffuse;
}

vec3 CalculateSpecular(
	const vec3 lightSpecular,
	const vec3 normal,
	const vec3 lightDirection,
	const vec3 viewDirection,
	const vec3 materialSpecular
)
{
	vec3 halhwayDirection = normalize(viewDirection + lightDirection);
	float specularFactor = pow(max(dot(halhwayDirection, normal), 0.0), uMaterial.shininess);
	vec3 specular = specularFactor * lightSpecular * materialSpecular;
	return specular;
}

float LinearizeDepth(float depth, float near, float far) 
{
    float z = depth * 2.0 - 1.0; // back to NDC 
    return ((2.0 * near * far) / (far + near - z * (far - near))) / far;	
}
