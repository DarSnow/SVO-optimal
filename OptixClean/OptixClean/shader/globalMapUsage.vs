#version 430 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

out VS_OUT 
{
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoords;
} vs_out;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

uniform bool reverse_normals;
//cell
vec3 faceNormal;
vec3 lightNormal;
vec2 finalUv;
vec3 pos;

//varying vec3 vPosDP;


uniform vec3 lightPos;

void main()
{

	//cell
	vec4 pos4 = vec4(aPos, 1);
	pos = (model * pos4).xyz;
	lightNormal = lightPos - pos;
	//vPosDP = (lightView * vec4(aPos, 1)).xyz;
	//cell
    vs_out.FragPos = vec3(model * vec4(aPos, 1.0));
    if(reverse_normals) // a slight hack to make sure the outer large cube displays lighting from the 'inside' instead of the default 'outside'.
        vs_out.Normal = transpose(inverse(mat3(model))) * (-1.0 * aNormal);
    else
        vs_out.Normal = transpose(inverse(mat3(model))) * aNormal;
    vs_out.TexCoords = aTexCoords;
	//pass
    gl_Position = projection * view * model * vec4(aPos, 1.0);
}