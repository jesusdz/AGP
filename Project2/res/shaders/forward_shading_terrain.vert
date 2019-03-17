#version 330 core

layout(location=0) in vec3 position;

uniform mat4 projectionMatrix;
uniform mat4 worldViewMatrix;

uniform float terrainSize;
uniform float terrainMaxHeight;
uniform vec2 terrainResolution;
uniform sampler2D terrainHeightMap;

out Data
{
    vec3 positionViewspace;
    vec3 normalLocalspace;
} VSOut;

void main(void)
{
    vec2 patchSize = vec2(terrainSize) / terrainResolution;
    vec2 texcoords = position.xz / terrainSize;
    vec2 incx = vec2(1.0, 0.0) / terrainResolution;
    vec2 incy = vec2(0.0, 1.0) / terrainResolution;

#define USE_TEXTURE_OFFSET
#ifdef USE_TEXTURE_OFFSET
    float hl = terrainMaxHeight * textureOffset(terrainHeightMap, texcoords, ivec2(-1, 0)).r;
    float hr = terrainMaxHeight * textureOffset(terrainHeightMap, texcoords, ivec2( 1, 0)).r;
    float hb = terrainMaxHeight * textureOffset(terrainHeightMap, texcoords, ivec2(0, -1)).r;
    float ht = terrainMaxHeight * textureOffset(terrainHeightMap, texcoords, ivec2(0,  1)).r;
#else
    float hl = terrainMaxHeight * texture(terrainHeightMap, texcoords - incx).r;
    float hr = terrainMaxHeight * texture(terrainHeightMap, texcoords + incx).r;
    float hb = terrainMaxHeight * texture(terrainHeightMap, texcoords - incy).r;
    float ht = terrainMaxHeight * texture(terrainHeightMap, texcoords + incy).r;
#endif

    float h = terrainMaxHeight * texture(terrainHeightMap, texcoords).r;
    vec3 modifiedPosition = position + vec3(0.0, h, 0.0);

    //vec3 normal = normalize(vec3((hl-hr)*0.5, 1.0/(terrainMaxHeight/5.0), (hb-ht)*0.5));
    vec3 normal = normalize(cross(vec3(0.0,ht-hb,patchSize.y), vec3(patchSize.x,hr-hl,0.0)));

    VSOut.positionViewspace = (worldViewMatrix * vec4(modifiedPosition, 1)).xyz;
    //VSOut.normalViewspace = (worldViewMatrix * vec4(normal, 0)).xyz;
    VSOut.normalLocalspace = normal;
    gl_Position = projectionMatrix * vec4(VSOut.positionViewspace, 1.0);
}
