#version 330 core

layout(location=0) in vec3 position;
layout(location=1) in vec3 normal;
layout(location=2) in vec2 texCoords;
layout(location=3) in vec3 tangent;
layout(location=4) in vec3 bitangent;

#define USE_INSTANCING
#ifdef USE_INSTANCING
layout(location=5) in mat4 aWorldMatrix;
layout(location=9) in mat3 aNormalMatrix;
out mat4 worldMatrix;
out mat3 normalMatrix;
#else
uniform mat4 worldMatrix;
#endif

uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;
uniform vec2 tiling;
uniform vec4 clippingPlane;


out Data
{
    vec3 positionWorldspace;
    vec3 normalWorldspace;
    vec2 texCoords;
} VSOut;

void main(void)
{
#ifdef USE_INSTANCING
    worldMatrix = aWorldMatrix;
    normalMatrix = aNormalMatrix;
#endif
    vec4 positionWorldspace = worldMatrix * vec4(position, 1);
    VSOut.positionWorldspace = positionWorldspace.xyz;
    vec2 offset = vec2(0.0);
    VSOut.texCoords = texCoords * tiling + offset;
    VSOut.normalWorldspace = vec3(worldMatrix * vec4(normal, 0.0));
    vec4 positionViewspace = viewMatrix * worldMatrix * vec4(position, 1);
    gl_Position = projectionMatrix * positionViewspace;

    // NOTE(jesus): Minimizes the artifact at water boundaries
    // At any point in the map, the clipping plane will be displaced
    // 1 meter towards the normal for each 100 meters we are away from that point.
    vec4 clipDistanceDisplacement = vec4(0.0, 0.0, 0.0, length(positionViewspace) / 100.0);

    gl_ClipDistance[0] = dot(vec4(positionWorldspace.xyz, 1.0), clippingPlane + clipDistanceDisplacement);
}
