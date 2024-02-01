#version 330 core

struct Material
{
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;

	float shininess;
};

struct Light
{
	vec3 position;

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

vec3 ComputeAmbient()
{
	vec3 ambient = uLight.ambient * uMaterial.ambient;
	return ambient;
}

vec3 ComputeDiffuse(const vec3 normal, const vec3 lightDirection)
{
	float diffuseFactor = max(dot(normal, lightDirection), 0.0);
	vec3 diffuse = diffuseFactor * uLight.diffuse * uMaterial.diffuse;
	return diffuse;
}

vec3 ComputeSpecular(const vec3 normal, const vec3 lightDirection)
{
	vec3 viewDirection = normalize(uViewPos - vWorldPos);
	vec3 halhwayDirection = normalize(viewDirection + lightDirection);
	float specularFactor = pow(max(dot(halhwayDirection, normal), 0.0), uMaterial.shininess);
	vec3 specular = specularFactor * uLight.specular * uMaterial.specular;
	return specular;
}

void main()
{
	vec3 normal = normalize(vNormal);
	vec3 lightDirection = normalize(uLight.position - vWorldPos);

	vec3 ambient = ComputeAmbient();
	vec3 diffuse = ComputeDiffuse(normal, lightDirection);
	vec3 specular = ComputeSpecular(normal, lightDirection);

	FragColor = vec4(ambient + diffuse + specular, 1.0f);
}
