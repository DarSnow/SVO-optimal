// Vertex shader:
// ================
#version 430 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texCoords;

uniform mat4 model;
uniform mat4 view;
uniform mat4 eyeMat;
uniform mat4 projection;

void main()
{
	vec4 pos = eyeMat*view*model*vec4(position,1.0f);
	gl_Position = projection*pos;
}