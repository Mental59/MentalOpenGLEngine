#version 330 core

out vec4 FragColor;

in vec2 vTexCoords;

uniform sampler2D uScreenTexture;
uniform sampler2D uBloomTexture;

const float offset = 1.0 / 600.0;
const vec2 offsets[9] = vec2[](
    vec2(-offset,  offset), // top-left
    vec2( 0.0f,    offset), // top-center
    vec2( offset,  offset), // top-right
    vec2(-offset,  0.0f),   // center-left
    vec2( 0.0f,    0.0f),   // center-center
    vec2( offset,  0.0f),   // center-right
    vec2(-offset, -offset), // bottom-left
    vec2( 0.0f,   -offset), // bottom-center
    vec2( offset, -offset)  // bottom-right    
);
const float kernel[9] = float[](
    -1.0, -1.0, -1.0,
    -1.0,  9.0, -1.0,
    -1.0, -1.0, -1.0
);

vec3 KernelEffect()
{
    vec3 col = vec3(0.0);

    for(int i = 0; i < 9; i++)
    {
        vec3 sampleTex = vec3(texture(uScreenTexture, vTexCoords + offsets[i]));
        col += sampleTex * kernel[i];
    }

    return col;
}

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
