#version 430 core
layout (points) in;
layout (line_strip, max_vertices = 2) out;

in VS_OUT {
	vec3 normal;
	vec3 color;
} gs_in[];

out vec3 gs_Color;

const float MAGNITUDE = 0.5;

void GenerateLine(int index)
{
    gl_Position = gl_in[index].gl_Position;
	gs_Color = gs_in[index].color;
    EmitVertex();
    gl_Position = gl_in[index].gl_Position + vec4(gs_in[index].normal, 0.0) * MAGNITUDE;
	gs_Color = gs_in[index].color;
	EmitVertex();
    EndPrimitive();
}

void main()
{
    GenerateLine(0);
}