#version 330 core

out vec4 FragColor;
in vec3 WorldPos;

uniform sampler2D equirectangularMap;
uniform float uGamma = 2.2;
uniform float uExposure = 1.0;

const vec2 invAtan = vec2(0.1591, 0.3183); // (1/(2*Pi), 1/Pi)
vec2 SampleSphericalMap(vec3 v) // mapping from a sphere onto uv-space
{
    vec2 uv = vec2(atan(v.z, v.x), asin(v.y));
    uv *= invAtan;
    uv += 0.5;
    return uv;
}


void main()
{		
    vec2 uv = SampleSphericalMap(normalize(WorldPos)); // normalize(WorldPos) is actually a mapping from cubemap to a sphere
    vec3 color = texture(equirectangularMap, uv).rgb;

    // exposure tone mapping
    vec3 mapped = vec3(1.0) - exp(-color * uExposure);
    // gamma correction 
    mapped = pow(mapped, vec3(1.0 / uGamma));
    
    FragColor = vec4(mapped, 1.0);
}
