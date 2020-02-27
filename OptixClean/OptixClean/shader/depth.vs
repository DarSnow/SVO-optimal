#version 430

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texCoords;

uniform mat4 model;
uniform mat4 viewprojection;

out vec4 position;
out vec2 texCoord;

void main()
{
    vec4 vertexPos = vec4(position, 1.0);
    position = viewprojection * model * vertexPos;
    texCoord = texCoords;
    // final drawing pos
    gl_Position = position;
}