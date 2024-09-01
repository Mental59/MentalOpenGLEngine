#version 330 core

layout (location = 0) in vec4 vertex;
out vec2 TexCoords;

uniform mat4 projection;

void main()
{
    vec2 pos = vec2(vertex.x, vertex.y);
    vec2 texCoords = vec2(vertex.z, vertex.w);

    gl_Position = projection * vec4(pos, 0.0, 1.0);
    TexCoords = texCoords;
}
