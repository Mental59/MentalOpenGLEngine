#version 330 core

out vec4 FragColor;

in vec3 vNormal;
in vec3 vWorldPos;

uniform vec3 uViewPos;
uniform samplerCube uSkybox;

void main()
{             
    vec3 I = normalize(vWorldPos - uViewPos);
    vec3 R = reflect(I, normalize(vNormal));
    FragColor = vec4(texture(uSkybox, R).rgb, 1.0);
}
