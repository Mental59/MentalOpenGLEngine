#version 330 core

out vec4 FragColor;

in vec2 vTexCoords;

uniform sampler2D uScreenTexture;
uniform sampler2D uBloomTexture;

uniform float uGamma = 2.2;
uniform float uExposure = 1.0;

void main()
{   
    vec3 screenTextureColor = texture(uScreenTexture, vTexCoords).rgb;
    vec3 bloomTextureColor = texture(uBloomTexture, vTexCoords).rgb;

//    screenTextureColor += bloomTextureColor; // additive blending
    
    //exposure tone mapping
    vec3 mapped = vec3(1.0) - exp(-screenTextureColor * uExposure);
    // gamma correction 
    mapped = pow(mapped, vec3(1.0 / uGamma));
    
    FragColor = vec4(mapped, 1.0);
}
