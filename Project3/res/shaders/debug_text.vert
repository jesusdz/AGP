#version 330 core

uniform mat4 projectionMatrix;

layout (location=0) in vec3 aPosition;
layout (location=1) in vec2 aTexCoord;

out vec2 vTexCoord;

void main()
{
    gl_Position = projectionMatrix * vec4(aPosition, 1.0);
    vTexCoord = aTexCoord;
}
