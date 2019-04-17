#version 330 core

// Camera position
uniform vec3 eyeWorldspace;

// Matrices
uniform mat4 worldMatrix;
uniform mat3 normalMatrix;

// Material
uniform vec4 albedo;
uniform vec4 specular;
uniform vec4 emissive;
uniform float smoothness;
uniform float metalness;
uniform float bumpiness;
uniform sampler2D albedoTexture;
uniform sampler2D specularTexture;
uniform sampler2D emissiveTexture;
uniform sampler2D normalTexture;
uniform sampler2D bumpTexture;

in Data
{
    vec3 positionWorldspace;
    vec3 normalLocalspace;
    vec2 texCoords;
    vec3 tangentLocalspace;
    vec3 bitangentLocalspace;
} FSIn;

layout(location=0) out vec4 rt0; // Albedo, AO
layout(location=1) out vec4 rt1; // Specular, roughness
layout(location=2) out vec4 rt2; // Normals
layout(location=3) out vec4 rt3; // Emissive + lightmaps



// Parallax occlusion mapping aka. relief mapping
vec2 reliefMapping(vec2 texCoords, mat3 TBN)
{
    int numSteps = 15;

    // Compute the ray in texture space
    mat3 TBNInverse = inverse(TBN);
    mat4 worldMatrixInverse = inverse(worldMatrix);
    vec3 rayWorldspace = normalize(FSIn.positionWorldspace - eyeWorldspace);
    vec3 rayTexspace = TBNInverse * mat3(worldMatrixInverse) * rayWorldspace;

    // Increment
    float heightScale = bumpiness;
    float texSize = max(textureSize(bumpTexture, 0).x, textureSize(bumpTexture, 0).y) * 2.0;
    vec3 rayIncrementTexspace;
    rayIncrementTexspace.xy = bumpiness * rayTexspace.xy / abs(rayTexspace.z * texSize);
    rayIncrementTexspace.z = 1.0/numSteps;

    // Sampling state
    vec3 samplePositionTexspace = vec3(texCoords, 0.0);
    float sampledDepth = 1.0;

    // Linear search
    for (int i = 0; i < numSteps && samplePositionTexspace.z < sampledDepth; ++i)
    {
        samplePositionTexspace += rayIncrementTexspace;
        sampledDepth = 1.0 - texture(bumpTexture, samplePositionTexspace.xy).r;
    }

    // Binary search
    for (int i = 0; i < 4; ++i)
    {
        rayIncrementTexspace *= 0.5;
        samplePositionTexspace += mix(
                    rayIncrementTexspace,
                    -rayIncrementTexspace,
                    float(samplePositionTexspace.z > sampledDepth));
        sampledDepth = 1.0 - texture(bumpTexture, samplePositionTexspace.xy).r;
    }

    return samplePositionTexspace.xy;
}

void main(void)
{
    vec2 texCoords = FSIn.texCoords;

    // Tangent to local (TBN) matrix
    vec3 T = FSIn.tangentLocalspace;
    vec3 B = FSIn.bitangentLocalspace;
    vec3 N = FSIn.normalLocalspace;
    mat3 TBN = mat3(T, B, N);

#define USE_RELIEF_MAPPING
#ifdef USE_RELIEF_MAPPING
    if (bumpiness > 0.0) {
        texCoords = reliefMapping(texCoords, TBN);
    }
#endif

#define USE_NORMAL_MAPPING
#ifdef USE_NORMAL_MAPPING
    // Normal in worldspace
    vec3 normalWorldspace = normalize(normalMatrix * FSIn.normalLocalspace);

    // Modified normal in worldspace
    vec3 tangentSpaceNormal = texture(normalTexture, texCoords).xyz * 2.0 - vec3(1.0);
    //tangentSpaceNormal *= tangentSpaceNormalsScaling;

    // For non-uniform mappings rescale the tangent space normal
    tangentSpaceNormal.x /= length(mat3(worldMatrix)*TBN*vec3(1, 0, 0));
    tangentSpaceNormal.y /= length(mat3(worldMatrix)*TBN*vec3(0, 1, 0));
    tangentSpaceNormal.z /= length(mat3(worldMatrix)*TBN*vec3(0, 0, 1));
    tangentSpaceNormal = normalize(tangentSpaceNormal);

    vec3 localSpaceNormal = TBN * tangentSpaceNormal;
    vec3 modifiedNormalWorldspace = normalize(mat3(worldMatrix) * localSpaceNormal);

    N = mix(
        normalWorldspace,
        modifiedNormalWorldspace,
        float(length(T) > 0.001));
#else
    // Normal without modifying in worldspace
    N = normalMatrix * FSIn.normalLocalspace;
#endif

    N = normalize(N);

    // albedo
    vec4 sampledAlbedo = pow(texture(albedoTexture, texCoords), vec4(2.2));
    if (sampledAlbedo.a < 0.01) { discard; }
    vec3 mixedAlbedo = albedo.rgb * sampledAlbedo.rgb;

    // specular
    vec3 sampledSpecular = pow(texture(specularTexture, texCoords).rgb, vec3(2.2));
    vec3 mixedSpecular = mix(vec3(0.0), sampledSpecular * mixedAlbedo, vec3(metalness));

    // Roughness
    float roughness = 1.0 - smoothness;

    // Emissive color
    rt0 = vec4(mixedAlbedo, 0.0);
    rt1 = vec4(mixedSpecular, roughness);
    rt2 = vec4(N*0.5 + vec3(0.5), 1.0);
    rt3 = vec4(emissive.rgb, 1.0);
}
