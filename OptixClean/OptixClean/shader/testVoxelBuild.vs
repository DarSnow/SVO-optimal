#version 430

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texCoords;

uniform mat4 model;
uniform mat4 view;
uniform mat4 eyeMat;
uniform mat4 projection;

out vec3 outposition;

void main(){
	vec4 pos = eyeMat*view*model*vec4(position,1.0f);
	gl_Position = projection*pos;

	outposition = gl_Position.xyz;
}