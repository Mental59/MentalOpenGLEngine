#version 330 core

out vec4 FragColor;

in vec2 vTexCoords;

uniform sampler2D uScreenTexture;
//uniform sampler2D uBloomTexture;

void main()
{   
    const float gamma = 2.2;
    const float exposure = 1.0;

    vec3 screenTextureColor = texture(uScreenTexture, vTexCoords).rgb;
//    vec3 bloomTextureColor = texture(uBloomTexture, vTexCoords).rgb;

//    screenTextureColor += bloomTextureColor; // additive blending
    
    vec3 mapped = vec3(1.0) - exp(-screenTextureColor * exposure);
    mapped = pow(mapped, vec3(1.0 / gamma));
    
    FragColor = vec4(mapped, 1.0);
}
