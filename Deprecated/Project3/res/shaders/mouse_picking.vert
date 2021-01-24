#version 330 core

layout(location=0) in vec3 position;

uniform mat4 worldMatrix;
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;

void main(void)
{
    gl_Position = projectionMatrix * viewMatrix * worldMatrix * vec4(position, 1);
}
