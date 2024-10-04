#version 330 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 color;
layout (location = 2) in vec2 texCoord;
out vec3 ourColor;
out vec2 TexCoord;

uniform float x_mov_full;
uniform float y_mov_full;

//out vec3 ourPosition;


void main()
{
gl_Position = vec4(position.x + x_mov_full, position.y + y_mov_full, position.z, 1.0);
ourColor = color;
TexCoord = texCoord;
//ourPosition = position;
}