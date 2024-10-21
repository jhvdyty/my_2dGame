#version 330 core
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aTexCoord;

out vec2 TexCoord;

uniform mat4 model;
uniform float aspectRatio;  // �������� ���

void main()
{
    vec4 pos = model * vec4(aPos.x, aPos.y * aspectRatio, 0.0, 1.0);  // ��������� ��������� �����
    gl_Position = pos;
    TexCoord = aTexCoord;
}