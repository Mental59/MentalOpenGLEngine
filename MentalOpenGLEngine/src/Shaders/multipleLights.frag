#version 330 core

struct Material
{
	sampler2D diffuse;
	sampler2D specular;
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
	const vec3 materialSpecular
);
vec3 CalculatePointLight(
	PointLight light,
	const vec3 normal,
	const vec3 viewDirection,
	const vec3 materialDiffuse,
	const vec3 materialSpecular
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

#define MAX_POINT_LIGHTS 4

out vec4 FragColor;

in vec2 vTexCoords;
in vec3 vNormal;
in vec3 vWorldPos;

uniform vec3 uViewPos;
uniform Material uMaterial;

uniform DirectionalLight uDirectionalLight;
uniform PointLight uPointLights[MAX_POINT_LIGHTS];
uniform SpotLight uSpotLight;
uniform int uNumPointLights;

void main()
{
	vec3 normal = normalize(vNormal);
	vec3 viewDirection = normalize(uViewPos - vWorldPos);
	vec3 materialDiffuse = texture(uMaterial.diffuse, vTexCoords).rgb;
	vec3 materialSpecular = texture(uMaterial.specular, vTexCoords).rgb;

	vec3 color = CalculateDirectionalLight(uDirectionalLight, normal, viewDirection, materialDiffuse, materialSpecular);

	for (int i = 0; i < min(MAX_POINT_LIGHTS, uNumPointLights); i++)
	{
		color += CalculatePointLight(uPointLights[i], normal, viewDirection, materialDiffuse, materialSpecular);
	}

	color += CalculateSpotLight(uSpotLight, normal, viewDirection, materialDiffuse, materialSpecular);

	FragColor = vec4(color, 1.0);
}

vec3 CalculateDirectionalLight(
	DirectionalLight light,
	const vec3 normal,
	const vec3 viewDirection,
	const vec3 materialDiffuse,
	const vec3 materialSpecular
)
{
	vec3 lightDirection = normalize(-light.direction);

	vec3 ambient = CalculateAmbient(light.ambient, materialDiffuse);
	vec3 diffuse = CalculateDiffuse(light.diffuse, normal, lightDirection, materialDiffuse);
	vec3 specular = CalculateSpecular(light.specular, normal, lightDirection, viewDirection, materialSpecular);

	return ambient + diffuse + specular;
}

vec3 CalculatePointLight(
	PointLight light,
	const vec3 normal,
	const vec3 viewDirection,
	const vec3 materialDiffuse,
	const vec3 materialSpecular
)
{
	vec3 lightDirection = normalize(light.position - vWorldPos);

	float distanceToLight = length(light.position - vWorldPos);
	float attenuation = 1.0 / (light.constant + light.linear * distanceToLight + light.quadratic * distanceToLight * distanceToLight);

	vec3 ambient = CalculateAmbient(light.ambient, materialDiffuse);
	vec3 diffuse = CalculateDiffuse(light.diffuse, normal, lightDirection, materialDiffuse);
	vec3 specular = CalculateSpecular(light.specular, normal, lightDirection, viewDirection, materialSpecular);

	return attenuation * (ambient + diffuse + specular);
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
	vec3 lightDirectionFromFragment = normalize(light.position - vWorldPos);

	float distanceToLight = length(light.position - vWorldPos);
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
