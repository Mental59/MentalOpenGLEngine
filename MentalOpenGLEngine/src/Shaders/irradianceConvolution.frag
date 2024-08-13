#version 330 core

out vec4 FragColor;
in vec3 WorldPos;

uniform samplerCube uEnvironmentMap;

const float PI = 3.14159265359;

vec3 computeIrradiance();

void main()
{
    vec3 irradiance = computeIrradiance();
    FragColor = vec4(irradiance, 1.0);
}

vec3 computeIrradiance()
{
    vec3 normal = normalize(WorldPos);

    vec3 irradiance = vec3(0.0);

    vec3 up = vec3(0.0, 1.0, 0.0);
    vec3 right = normalize(cross(up, normal));
    up = normalize(cross(normal, right));

    float sampleDelta = 0.025;
    int nrSamples = 0; 
    for(float phi = 0.0; phi < 2.0 * PI; phi += sampleDelta)
    {
        for(float theta = 0.0; theta < 0.5 * PI; theta += sampleDelta)
        {
            // spherical to cartesian (in tangent space)
            vec3 tangentSample = vec3(sin(theta) * cos(phi),  sin(theta) * sin(phi), cos(theta));
            // tangent space to world
            vec3 sampleVec = tangentSample.x * right + tangentSample.y * up + tangentSample.z * normal; 

            irradiance += texture(uEnvironmentMap, sampleVec).rgb * cos(theta) * sin(theta);
            nrSamples++;
        }
    }

    irradiance = PI * irradiance / float(nrSamples);
    return irradiance;
}
