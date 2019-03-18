#version 330 core

layout(location=0) in vec3 position;
layout(location=1) in vec3 normal;
layout(location=2) in vec2 texCoords;
layout(location=3) in vec3 tangent;
layout(location=4) in vec3 bitangent;

uniform mat4 projectionMatrix;
uniform mat4 worldViewMatrix;

out Data
{
    vec3 positionViewspace;
    vec3 normalLocalspace;
    vec2 texCoords;
    vec3 tangentLocalspace;
    vec3 bitangentLocalspace;
} VSOut;

void main(void)
{
    VSOut.positionViewspace = (worldViewMatrix * vec4(position, 1)).xyz;
    VSOut.normalLocalspace = normal;
    VSOut.texCoords = texCoords;
    VSOut.tangentLocalspace = tangent;
    VSOut.bitangentLocalspace = bitangent;
    gl_Position = projectionMatrix * vec4(VSOut.positionViewspace, 1.0);
}
