#version 430 core
layout (location = 0) in vec3 aPos;

uniform mat4 model;
uniform mat4 lightView;

uniform float direction;

varying vec2 depth;
varying float clip;

uniform float near_plane;
uniform float far_plane;

void main()
{

	//the object's world multiplied with the light view
    //highp vec4 pos4 = worldView * vec4(position, 1);

	vec4 pos4 = lightView * model * vec4(aPos, 1.0);

    pos4 = pos4 / pos4.w;
    pos4.z = pos4.z * direction;

    float L = length(pos4.xyz);
	//标准化
    pos4 = pos4 / L;

    clip = pos4.z;
	
    pos4.z = pos4.z + 1;
    pos4.x = pos4.x / pos4.z;
    pos4.y = pos4.y / pos4.z;

	//整理到near-far标准内
	//当前也仅计算出一个坐标，将z填入
	//不用near了？
    pos4.z = L / (far_plane);
	//pos4.z -= 0.8;
    pos4.w = 1;

    depth = pos4.zw;

    gl_Position = pos4;
}