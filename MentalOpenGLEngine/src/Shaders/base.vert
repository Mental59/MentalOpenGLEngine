#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

out vec2 vTexCoords;
out vec3 vLightColor;

uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProjection;

uniform vec3 uLightColor;
uniform vec3 uLightPos;
uniform vec3 uViewPos;

void main()
{
	vec3 worldPos = vec3(uModel * vec4(aPos, 1.0));

	vec3 normal = transpose(inverse(mat3(uModel))) * aNormal;

	vTexCoords = aTexCoords;

	float ambientStrength = 0.1;
	vec3 ambient = ambientStrength * uLightColor;

	vec3 lightDirection = normalize(uLightPos - worldPos);

	float diffuseFactor = max(dot(normal, lightDirection), 0.0);
	vec3 diffuse = diffuseFactor * uLightColor;

	float specularStrength = 1.0;
	float specularHighlightShininess = 32;
	vec3 viewDirection = normalize(uViewPos - worldPos);
	vec3 reflectDirection = reflect(-lightDirection, normal);
	float specularFactor = pow(max(dot(viewDirection, reflectDirection), 0.0), specularHighlightShininess);
	vec3 specular = specularStrength * specularFactor * uLightColor;
	
	vLightColor = ambient + diffuse + specular;

	gl_Position = uProjection * uView * vec4(worldPos, 1.0);
}
