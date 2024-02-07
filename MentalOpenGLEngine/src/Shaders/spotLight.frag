#version 330 core

struct Material
{
	sampler2D diffuse;
	sampler2D specular;
	float shininess;
};

struct Light
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

out vec4 FragColor;

in vec2 vTexCoords;
in vec3 vNormal;
in vec3 vWorldPos;

uniform vec3 uViewPos;
uniform Material uMaterial;
uniform Light uLight;

vec3 ComputeAmbient()
{
	vec3 ambient = uLight.ambient * texture(uMaterial.diffuse, vTexCoords).rgb;
	return ambient;
}

vec3 ComputeDiffuse(const vec3 normal, const vec3 lightDirection)
{
	float diffuseFactor = max(dot(normal, lightDirection), 0.0);
	vec3 diffuse = diffuseFactor * uLight.diffuse * texture(uMaterial.diffuse, vTexCoords).rgb;
	return diffuse;
}

vec3 ComputeSpecular(const vec3 normal, const vec3 lightDirection)
{
	vec3 viewDirection = normalize(uViewPos - vWorldPos);
	vec3 halhwayDirection = normalize(viewDirection + lightDirection);
	float specularFactor = pow(max(dot(halhwayDirection, normal), 0.0), uMaterial.shininess);
	vec3 specular = specularFactor * uLight.specular * texture(uMaterial.specular, vTexCoords).rgb;
	return specular;
}

void main()
{
	float distanceToLight = length(uLight.position - vWorldPos);
	float attenuation = 1.0 / (uLight.constant + uLight.linear * distanceToLight + uLight.quadratic * distanceToLight * distanceToLight);

	vec3 normal = normalize(vNormal);
	vec3 lightDirectionFromFragment = normalize(uLight.position - vWorldPos);

	vec3 ambient = ComputeAmbient();
	vec3 diffuse = ComputeDiffuse(normal, lightDirectionFromFragment);
	vec3 specular = ComputeSpecular(normal, lightDirectionFromFragment);

	float theta = dot(lightDirectionFromFragment, normalize(-uLight.direction));
	float epsilon = uLight.cutOffCosine - uLight.outerCutOffCosine;
	float lightIntensity = clamp((theta - uLight.outerCutOffCosine) / epsilon, 0.0, 1.0);
	
	FragColor = vec4(ambient + attenuation * lightIntensity * (diffuse + specular), 1.0f);
}
