#version 430 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec3 color;

out VS_OUT {
    vec3 normal;
	vec3 color;
} vs_out;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
	vs_out.color = color;
	mat3 normalMatrix = mat3(transpose(inverse(view * model)));
	vs_out.normal = vec3(projection * vec4(normalMatrix * normal, 0.0));
	gl_Position = projection * view * model * vec4(position, 1.0f);
}