#version 330 core

uniform vec4 backgroundColor;

out vec4 outColor;

void main()
{
    outColor = pow(backgroundColor, vec4(2.2));
    outColor.a = 1.0;
}
