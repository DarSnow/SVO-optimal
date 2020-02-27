#version 430 core
layout (location = 0) in vec3 aPos;

uniform mat4 model;
uniform mat4 vplView;

uniform float direction;
uniform float near_plane;
uniform float far_plane;

varying vec2 depth;
varying float clip;


void main()
{
	vec4 pos4 = vplView * model * vec4(aPos, 1.0);

    pos4 = pos4 / pos4.w;
    pos4.z = pos4.z * direction;

    float L = length(pos4.xyz);
	//±ê×¼»¯
    pos4 = pos4 / L;

    clip = pos4.z;
	
    pos4.z = pos4.z + 1;
    pos4.x = pos4.x / pos4.z;
    pos4.y = pos4.y / pos4.z;
    pos4.z = L / (far_plane);
    pos4.w = 1;

    depth = pos4.zw;

    gl_Position = pos4;
}