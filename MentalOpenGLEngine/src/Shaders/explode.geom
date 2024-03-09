#version 330 core

layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

uniform float uTime;

in VS_OUT {
    vec2 texCoords;
	vec3 normal;
	vec3 worldPos;
} gs_in[];

out VS_OUT {
    vec2 texCoords;
	vec3 normal;
	vec3 worldPos;
} gs_out;

vec3 ComputeNormal() // NDC is in left-handed coordinate system
{
    vec3 a = vec3(gl_in[0].gl_Position) - vec3(gl_in[1].gl_Position);
    vec3 b = vec3(gl_in[2].gl_Position) - vec3(gl_in[1].gl_Position);
    return normalize(cross(a, b));
}

vec4 Explode(vec4 position, vec3 normal)
{
    float magnitude = 2.0;
    vec3 direction = ((sin(uTime) + 1.0) / 2.0) * magnitude * normal; 
    return position + vec4(direction, 0.0);
}

void main()
{    
    vec3 normal = ComputeNormal();

    for (int i = 0; i < 3; i++)
    {
        gl_Position = Explode(gl_in[i].gl_Position, normal);
        gs_out.texCoords = gs_in[i].texCoords;
        gs_out.normal = gs_in[i].normal;
        gs_out.worldPos = gs_in[i].worldPos;
        EmitVertex();
    }
    EndPrimitive();
}
