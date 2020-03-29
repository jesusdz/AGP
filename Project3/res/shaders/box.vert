#version 330 core

uniform mat4 worldMatrix;
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;

uniform vec3 boundsMin;
uniform vec3 boundsMax;

layout (location=0) in vec3 position;

void main()
{
    vec3 newPosition = boundsMin + position * (boundsMax - boundsMin);
    gl_Position = projectionMatrix * viewMatrix * worldMatrix * vec4(newPosition, 1.0);
}
