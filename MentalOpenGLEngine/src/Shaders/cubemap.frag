#version 330 core

out vec4 FragColor;

in vec3 vTexCoords;

uniform samplerCube uSkybox;

uniform float uGamma = 1.5;
uniform float uExposure = 1.0;

void main()
{
    vec3 envColor = texture(uSkybox, vTexCoords).rgb;

    // exposure tone mapping
    vec3 mapped = vec3(1.0) - exp(-envColor * uExposure);
    // gamma correction 
    mapped = pow(mapped, vec3(1.0 / uGamma));

    FragColor = vec4(mapped, 1.0);
}
