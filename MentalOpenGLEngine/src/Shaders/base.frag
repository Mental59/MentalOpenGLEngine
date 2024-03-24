#version 330 core

struct Material
{
	sampler2D diffuseTexture1;
	sampler2D specularTexture1;
	float shininess;
};

struct DirectionalLight
{
	vec3 direction;
  
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
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

#define MAX_POINT_LIGHTS 4

out vec4 FragColor;

in VS_OUT {
    vec2 texCoords;
	vec3 normal;
	vec3 worldPos;
	vec4 posInLightSpace;
} fs_in;

uniform vec3 uViewPos;
uniform Material uMaterial;
uniform DirectionalLight uDirLight;
uniform PointLight uPointLights[MAX_POINT_LIGHTS];
uniform int uNumPointLights;
uniform sampler2D uShadowMap;
uniform samplerCube uShadowCubeMap;

vec3 CalculateDirectionalLight(
	DirectionalLight light,
	const vec3 normal,
	const vec3 viewDirection,
	const vec3 materialDiffuse,
	const vec3 materialSpecular,
	const float shadow
);
vec3 CalculatePointLight(
	PointLight light,
	const vec3 normal,
	const vec3 viewDirection,
	const vec3 materialDiffuse,
	const vec3 materialSpecular,
	const float shadow
);
vec3 CalculateSpotLight(
	SpotLight light,
	const vec3 normal,
	const vec3 viewDirection,
	const vec3 materialDiffuse,
	const vec3 materialSpecular
);
vec3 CalculateAmbient(
	const vec3 lightAmbient,
	const vec3 materialDiffuse
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

float CalculateDirShadow(const vec3 normal, const vec3 lightDirection);
float CalculatePointShadow(const vec3 lightPos, const float farPlane);

const int numSamples = 20;
vec3 gridSamplingDisk[numSamples] = vec3[]
(
   vec3(1, 1,  1), vec3( 1, -1,  1), vec3(-1, -1,  1), vec3(-1, 1,  1), 
   vec3(1, 1, -1), vec3( 1, -1, -1), vec3(-1, -1, -1), vec3(-1, 1, -1),
   vec3(1, 1,  0), vec3( 1, -1,  0), vec3(-1, -1,  0), vec3(-1, 1,  0),
   vec3(1, 0,  1), vec3(-1,  0,  1), vec3( 1,  0, -1), vec3(-1, 0, -1),
   vec3(0, 1,  1), vec3( 0, -1,  1), vec3( 0, -1, -1), vec3( 0, 1, -1)
);

void main()
{
	vec4 diffuseColor = texture(uMaterial.diffuseTexture1, fs_in.texCoords);

//	if (diffuseColor.a < 0.1)
//	{
//		discard;
//	}

	vec3 normal = normalize(fs_in.normal);
	vec3 viewDirection = normalize(uViewPos - fs_in.worldPos);
	vec3 materialDiffuse = diffuseColor.rgb;
	vec3 materialSpecular = texture(uMaterial.specularTexture1, fs_in.texCoords).rgb;
	float shadow = CalculateDirShadow(normal, normalize(-uDirLight.direction));

	vec3 depth = vec3(LinearizeDepth(gl_FragCoord.z, 0.1, 100.0));

	vec3 color = CalculateDirectionalLight(uDirLight, normal, viewDirection, materialDiffuse, materialSpecular, shadow);
	for (int i = 0; i < min(MAX_POINT_LIGHTS, uNumPointLights); i++)
	{
		shadow = CalculatePointShadow(uPointLights[i].position, uPointLights[i].farPlane);
		color += CalculatePointLight(uPointLights[i], normal, viewDirection, materialDiffuse, materialSpecular, shadow);
	}

	FragColor = vec4(color, 1.0);
}

float CalculateDirShadow(const vec3 normal, const vec3 lightDirection)
{
	// perform perspective division, so it works for perspective matrices too
    vec3 projCoords = fs_in.posInLightSpace.xyz / fs_in.posInLightSpace.w;

	// conversion to the range [0, 1]
	projCoords = projCoords * 0.5 + 0.5; 

//	float closestDepth = texture(uShadowMap, projCoords.xy).r;
	float currentDepth = projCoords.z;

	float bias = max(0.005 * (1.0 - dot(normal, lightDirection)), 0.0005);

	// PCF
	float shadow = 0.0;
	vec2 texelSize = 1.0 / textureSize(uShadowMap, 0);
	for (int x = -2; x <= 2; x++)
	{
		for (int y = -2; y <= 2; y++)
		{
			float pcfDepth = texture(uShadowMap, projCoords.xy + vec2(x, y) * texelSize).r;
			shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;
		}
	}
	shadow /= 25.0;

	return shadow;
}

float CalculatePointShadow(const vec3 lightPos, const float farPlane)
{
	vec3 fragToLight = fs_in.worldPos - lightPos;
	float currentDepth = length(fragToLight);
	float shadow = 0.0;
	float bias   = 0.05;
	float viewDistance = length(uViewPos - fs_in.worldPos);
	float diskRadius = (1.0 + (viewDistance / farPlane)) / 50.0;  
	for(int i = 0; i < numSamples; i++)
	{
		float closestDepth = texture(uShadowCubeMap, fragToLight + gridSamplingDisk[i] * diskRadius).r;
		closestDepth *= farPlane;   // undo mapping [0;1]
		if(currentDepth - bias > closestDepth)
			shadow += 1.0;
	}
	shadow /= float(numSamples); 

//	float closestDepth = texture(uShadowCubeMap, fragToLight).r;
//	FragColor = vec4(vec3(closestDepth), 1.0);

    return shadow;
}

vec3 CalculateDirectionalLight(
	DirectionalLight light,
	const vec3 normal,
	const vec3 viewDirection,
	const vec3 materialDiffuse,
	const vec3 materialSpecular,
	const float shadow
)
{
	vec3 lightDirection = normalize(-light.direction);

	vec3 ambient = CalculateAmbient(light.ambient, materialDiffuse);
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
	const float shadow
)
{
	vec3 lightDirection = normalize(light.position - fs_in.worldPos);

	float distanceToLight = length(light.position - fs_in.worldPos);
	float attenuation = 1.0 / (light.constant + light.linear * distanceToLight + light.quadratic * distanceToLight * distanceToLight);

	vec3 ambient = CalculateAmbient(light.ambient, materialDiffuse);
	vec3 diffuse = CalculateDiffuse(light.diffuse, normal, lightDirection, materialDiffuse);
	vec3 specular = CalculateSpecular(light.specular, normal, lightDirection, viewDirection, materialSpecular);

	return attenuation * (ambient + (1.0 - shadow) * (diffuse + specular));
}

vec3 CalculateSpotLight(
	SpotLight light,
	const vec3 normal,
	const vec3 viewDirection,
	const vec3 materialDiffuse,
	const vec3 materialSpecular
)
{
	vec3 lightDirection = normalize(-light.direction);
	vec3 lightDirectionFromFragment = normalize(light.position - fs_in.worldPos);

	float distanceToLight = length(light.position - fs_in.worldPos);
	float attenuation = 1.0 / (light.constant + light.linear * distanceToLight + light.quadratic * distanceToLight * distanceToLight);

	float theta = dot(lightDirectionFromFragment, normalize(-light.direction));
	float epsilon = light.cutOffCosine - light.outerCutOffCosine;
	float lightIntensity = clamp((theta - light.outerCutOffCosine) / epsilon, 0.0, 1.0);

	vec3 ambient = CalculateAmbient(light.ambient, materialDiffuse);
	vec3 diffuse = CalculateDiffuse(light.diffuse, normal, lightDirection, materialDiffuse);
	vec3 specular = CalculateSpecular(light.specular, normal, lightDirection, viewDirection, materialSpecular);

	return lightIntensity * attenuation * (ambient + diffuse + specular); 
}

vec3 CalculateAmbient(
	const vec3 lightAmbient,
	const vec3 materialDiffuse
)
{
	vec3 ambient = lightAmbient * materialDiffuse;
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
