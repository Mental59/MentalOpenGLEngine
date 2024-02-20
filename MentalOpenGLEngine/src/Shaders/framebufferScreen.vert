#version 330 core

layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aTexCoords;

out vec2 vTexCoords;

uniform mat4 uModelMat;

void main()
{
    vTexCoords = aTexCoords;
    gl_Position = uModelMat * vec4(aPos, 0.0, 1.0); 
}  
