#version 330 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 color;
layout (location = 2) in vec2 aTexCoord;

uniform mat4 transform;

out vec3 ourColor;
out vec2 TexCoord;

uniform vec4 texCoords;

void main()
{
    gl_Position = transform * vec4(position, 1.0f);
    ourColor = color;
    //TexCoord = aTexCoord;
    TexCoord = vec2(
        texCoords.x + (texCoords.z - texCoords.x) * aTexCoord.x,
        texCoords.y + (texCoords.w - texCoords.y) * aTexCoord.y
    );
}