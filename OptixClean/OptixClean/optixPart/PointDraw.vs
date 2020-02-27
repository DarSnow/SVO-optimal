#version 430 core
layout (location = 0) in vec4 aPos;

uniform mat4 transform;
uniform int sampleGap;
out float valid;
out vec4  color;
float pricision = 0.1;
vec4 red = vec4(1,0,0,1);
vec4 green = vec4(0,1,0,1);
vec4 blue = vec4(0,0,1,1);

void main(){
	valid = 1.0f;
	if(abs(aPos.x)<pricision && abs(aPos.y)<pricision && abs(aPos.z)<pricision){
		valid = 0.0f;
	}
	if(gl_VertexID<sampleGap){
		color = red;
	}
	else if(gl_VertexID<sampleGap*2){
		color = green;
	}
	else{
		color = blue;
	}
    gl_Position = transform * aPos; 
}