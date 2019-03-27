#version 330 core

layout(location=0) in vec3 position;
layout(location=1) in vec3 normal;
layout(location=2) in vec2 texCoords;
layout(location=3) in vec3 tangent;
layout(location=4) in vec3 bitangent;

uniform mat4 worldMatrix;
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;
uniform vec2 tiling;


out Data
{
    vec3 positionWorldspace;
    vec3 normalLocalspace;
    vec2 texCoords;
    vec3 tangentLocalspace;
    vec3 bitangentLocalspace;
} VSOut;

void main(void)
{
    vec3 positionWorldspace = vec3(worldMatrix * vec4(position, 1));
    VSOut.positionWorldspace = positionWorldspace;
    vec2 offset = vec2(0.0);
    VSOut.texCoords = texCoords * tiling + offset;
    VSOut.tangentLocalspace = tangent / tiling.x;
    VSOut.bitangentLocalspace = bitangent / tiling.y;
    float tangentScale = mix(length(VSOut.tangentLocalspace), length(VSOut.bitangentLocalspace), 0.5);
    VSOut.normalLocalspace = normal * mix(1.0, tangentScale, tangentScale > 0);
    gl_Position = projectionMatrix * viewMatrix * vec4(positionWorldspace, 1.0);
}
