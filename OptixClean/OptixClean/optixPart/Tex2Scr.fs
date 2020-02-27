#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D scrTexture;

void main()
{
    vec3 scrcol = texture(scrTexture, TexCoords).rgb;
    FragColor = vec4(scrcol, 1.0);
}