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

in VS_OUT {
    vec2 texCoords;
	vec3 normal;
	vec3 worldPos;
} fs_in;

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
	vec3 viewDirection = normalize(uViewPos - fs_in.worldPos);
	vec3 halhwayDirection = normalize(viewDirection + lightDirection);
	float specularFactor = pow(max(dot(halhwayDirection, normal), 0.0), uMaterial.shininess);
	vec3 specular = specularFactor * uLight.specular * specularColor;
	return specular;
}
  
float LinearizeDepth(float depth, float near, float far) 
{
    float z = depth * 2.0 - 1.0; // back to NDC 
    return ((2.0 * near * far) / (far + near - z * (far - near))) / far;	
}

void main()
{
	vec4 diffuseColor = texture(uMaterial.diffuseTexture1, fs_in.texCoords);

	if (diffuseColor.a < 0.1)
	{
		discard;
	}

	vec3 normal = normalize(fs_in.normal);
	vec3 lightDirection = normalize(-uLight.direction);
	
    vec3 specularColor = texture(uMaterial.specularTexture1, fs_in.texCoords).rrr;

	vec3 ambient = ComputeAmbient(diffuseColor.rgb);
	vec3 diffuse = ComputeDiffuse(normal, lightDirection, diffuseColor.rgb);
	vec3 specular = ComputeSpecular(normal, lightDirection, specularColor);

	vec3 depth = vec3(LinearizeDepth(gl_FragCoord.z, 0.1, 100.0));

	FragColor = vec4(ambient + diffuse + specular + depth, 1.0);
}
