#version 330 core

out vec4 FragColor;
in vec3 ourColor;
in vec2 TexCoord;

uniform sampler2D ourTexture1;

void main()
{
    vec4 texColor = texture(ourTexture1, TexCoord);
    if(texColor.a < 0.1)
        discard;
    FragColor = texColor;
}

//#version 330 core
//in vec3 ourColor;
//in vec2 TexCoord;
//
//out vec4 FragColor;
//
//uniform sampler2D ourTexture1;
//
//void main()
//{
//    FragColor = texture(ourTexture1, TexCoord);
//}