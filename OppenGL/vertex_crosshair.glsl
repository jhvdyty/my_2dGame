#version 330 core
layout (location = 0) in vec3 position;
uniform vec2 crosshairPosition;

void main()
{
    vec3 pos = vec3(position.x + crosshairPosition.x, position.y + crosshairPosition.y, 0.0);
    gl_Position = vec4(pos, 1.0);
}
