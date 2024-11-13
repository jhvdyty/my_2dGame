#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;
layout (location = 2) in vec2 aTexCoord;

out vec3 ourColor;
out vec2 TexCoord;

uniform float y_mov;
uniform float x_mov;

void main()
{
    gl_Position = vec4(aPos.x + x_mov, aPos.y + y_mov, aPos.z, 1.0);
    ourColor = aColor;
    TexCoord = aTexCoord;
}