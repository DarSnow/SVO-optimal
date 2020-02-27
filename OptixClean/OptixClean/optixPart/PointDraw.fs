#version 430 core
out vec4 FragColor;

in float valid;
in vec4 color;

void main()
{
	if(valid < 0.5f){
		discard;
	}
    FragColor = color;
}