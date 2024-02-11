#version 330 core

struct Material
{
	sampler2D diffuseTexture1;
	sampler2D specularTexture1;
	float shininess;
};

struct Light
{
	vec3 direction;
	vec3 ambient;
	vec3 diffuse;
	vec3 specular; 
};

out vec4 FragColor;

in vec2 vTexCoords;
in vec3 vNormal;
in vec3 vWorldPos;

uniform vec3 uViewPos;
uniform Material uMaterial;
uniform Light uLight;

vec3 ComputeAmbient(const vec3 diffuseColor)
{
	vec3 ambient = uLight.ambient * diffuseColor;
	return ambient;
}

vec3 ComputeDiffuse(const vec3 normal, const vec3 lightDirection, const vec3 diffuseColor)
{
	float diffuseFactor = max(dot(normal, lightDirection), 0.0);
	vec3 diffuse = diffuseFactor * uLight.diffuse * diffuseColor;
	return diffuse;
}

vec3 ComputeSpecular(const vec3 normal, const vec3 lightDirection, const vec3 specularColor)
{
	vec3 viewDirection = normalize(uViewPos - vWorldPos);
	vec3 halhwayDirection = normalize(viewDirection + lightDirection);
	float specularFactor = pow(max(dot(halhwayDirection, normal), 0.0), uMaterial.shininess);
	vec3 specular = specularFactor * uLight.specular * specularColor;
	return specular;
}

void main()
{
	vec3 normal = normalize(vNormal);
	vec3 lightDirection = normalize(-uLight.direction);

	vec3 diffuseColor = texture(uMaterial.diffuseTexture1, vTexCoords).rgb;
	vec3 specularColor = texture(uMaterial.specularTexture1, vTexCoords).rgb;

	vec3 ambient = ComputeAmbient(diffuseColor);
	vec3 diffuse = ComputeDiffuse(normal, lightDirection, diffuseColor);
	vec3 specular = ComputeSpecular(normal, lightDirection, specularColor);

	FragColor = vec4(ambient + diffuse + specular, 1.0f);
}
