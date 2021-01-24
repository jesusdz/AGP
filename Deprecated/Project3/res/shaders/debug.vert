#version 330 core

uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;

layout (location=0) in vec3 aPosition;
layout (location=1) in vec3 aColor;

out vec3 color;

void main()
{
    gl_Position = projectionMatrix * viewMatrix * vec4(aPosition, 1.0);
    color = aColor;
}
