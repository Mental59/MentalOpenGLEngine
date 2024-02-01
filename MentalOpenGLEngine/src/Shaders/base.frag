#version 330 core

out vec4 FragColor;

in vec2 vTexCoords;
in vec3 vNormal;
in vec3 vWorldPos;

uniform vec3 uObjectColor;
uniform vec3 uLightColor;
uniform vec3 uLightPos;
uniform vec3 uViewPos;

void main()
{
	float ambientStrength = 0.1;
	vec3 ambient = ambientStrength * uLightColor;

	vec3 normal = normalize(vNormal);
	vec3 lightDirection = normalize(uLightPos - vWorldPos);

	float diffuseFactor = max(dot(normal, lightDirection), 0.0);
	vec3 diffuse = diffuseFactor * uLightColor;

	float specularStrength = 0.5;
	float specularHighlightShininess = 32 * 4;
	vec3 viewDirection = normalize(uViewPos - vWorldPos);
	vec3 halhwayDirection = normalize(viewDirection + lightDirection);
	float specularFactor = pow(max(dot(halhwayDirection, normal), 0.0), specularHighlightShininess);
	vec3 specular = specularStrength * specularFactor * uLightColor;
	
	vec3 finalColor = (ambient + diffuse + specular) * uObjectColor;

	FragColor = vec4(finalColor, 1.0f);
}
