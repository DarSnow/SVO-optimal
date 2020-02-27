#version 430 core

out vec4 color;

in vec3 gs_Color;


void main()
{
	color = vec4(gs_Color,1);
}