#version 330 core

layout (triangles) in;
layout (line_strip, max_vertices = 6) out;

in VS_OUT {
    vec3 normal;
} gs_in[];

const float MAGNITUDE = 0.05;

uniform mat4 uProjection;

out vec3 vColor;

void GenerateLine(int index, const vec3 color)
{
    vColor = color;

    gl_Position = uProjection * gl_in[index].gl_Position;
    EmitVertex();

    gl_Position = uProjection * (gl_in[index].gl_Position + normalize(vec4(gs_in[index].normal, 0.0)) * MAGNITUDE);
    EmitVertex();

    EndPrimitive();
}

void main()
{
//    for (int i = 0; i < 3; i++)
//    {
//        GenerateLine(i);
//    }

    GenerateLine(0, vec3(1.0, 0.0, 0.0));
    GenerateLine(1, vec3(0.0, 1.0, 0.0));
    GenerateLine(2, vec3(0.0, 0.0, 1.0));
}
