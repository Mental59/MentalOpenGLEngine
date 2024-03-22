#version 330 core

layout (triangles) in;
layout (triangle_strip, max_vertices=18) out;

uniform mat4 uShadowMatrices[6];

out vec4 FragPos;

void main()
{
    for (int face = 0; face < 6; face++)
    {
        gl_Layer = face; // built-in variable that specifies to which face we render (with this we specify a face of a cubemap).
        for (int i = 0; i < 3; i++)
        {
            FragPos = gl_in[i].gl_Position;
            gl_Position = uShadowMatrices[face] * FragPos;
            EmitVertex();
        }    
        EndPrimitive();
    }
}
