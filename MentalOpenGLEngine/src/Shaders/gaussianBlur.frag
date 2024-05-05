#version 330 core

out vec4 FragColor;

in vec2 vTexCoords;

uniform sampler2D uImage;

uniform bool uHorizontal;
uniform float uWeights[5] = float[] (0.2270270270, 0.1945945946, 0.1216216216, 0.0540540541, 0.0162162162);

void main()
{             
     vec2 tex_offset = 1.0 / textureSize(uImage, 0); // gets size of single texel

     vec3 result = texture(uImage, vTexCoords).rgb * uWeights[0];

     if (uHorizontal)
     {
         for(int i = 1; i < 5; ++i)
         {
            result += texture(uImage, vTexCoords + vec2(tex_offset.x * i, 0.0)).rgb * uWeights[i];
            result += texture(uImage, vTexCoords - vec2(tex_offset.x * i, 0.0)).rgb * uWeights[i];
         }
     }
     else
     {
         for(int i = 1; i < 5; ++i)
         {
             result += texture(uImage, vTexCoords + vec2(0.0, tex_offset.y * i)).rgb * uWeights[i];
             result += texture(uImage, vTexCoords - vec2(0.0, tex_offset.y * i)).rgb * uWeights[i];
         }
     }
     FragColor = vec4(result, 1.0);
}
