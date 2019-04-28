#version 330 core

// Camera position
uniform vec3 eyeWorldspace;

// Matrices
#define USE_INSTANCING
#ifdef USE_INSTANCING
in mat4 worldMatrix;
in mat3 normalMatrix;
#else
uniform mat4 worldMatrix;
uniform mat3 normalMatrix;
#endif

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

layout(location=0) out vec4 outColor;



void main(void)
{
    vec2 texCoords = FSIn.texCoords;

//    // Tangent to local (TBN) matrix
//    vec3 T = FSIn.tangentLocalspace;
//    vec3 B = FSIn.bitangentLocalspace;
//    vec3 N = FSIn.normalLocalspace;
//    mat3 TBN = mat3(T, B, N);
//
////#define USE_NORMAL_MAPPING
//#ifdef USE_NORMAL_MAPPING
//    // Normal in worldspace
//    vec3 normalWorldspace = normalize(normalMatrix * FSIn.normalLocalspace);
//
//    // Modified normal in worldspace
//    vec3 tangentSpaceNormal = texture(normalTexture, texCoords).xyz * 2.0 - vec3(1.0);
//    //tangentSpaceNormal *= tangentSpaceNormalsScaling;
//
//    // For non-uniform mappings rescale the tangent space normal
//    tangentSpaceNormal.x /= length(mat3(worldMatrix)*TBN*vec3(1, 0, 0));
//    tangentSpaceNormal.y /= length(mat3(worldMatrix)*TBN*vec3(0, 1, 0));
//    tangentSpaceNormal.z /= length(mat3(worldMatrix)*TBN*vec3(0, 0, 1));
//    tangentSpaceNormal = normalize(tangentSpaceNormal);
//
//    vec3 localSpaceNormal = TBN * tangentSpaceNormal;
//    vec3 modifiedNormalWorldspace = normalize(mat3(worldMatrix) * localSpaceNormal);
//
//    N = length(T) < 0.001 ? normalWorldspace : modifiedNormalWorldspace;
//#else
//    // Normal without modifying in worldspace
//    N = normalMatrix * FSIn.normalLocalspace;
//#endif
//
//    N = normalize(N);

    // albedo
    vec4 sampledAlbedo = pow(texture(albedoTexture, texCoords), vec4(2.2));
    if (sampledAlbedo.a < 0.01) { discard; }
    vec3 mixedAlbedo = albedo.rgb * sampledAlbedo.rgb;

//    // specular / reflectance
//    vec3 sampledSpecular = pow(texture(specularTexture, texCoords).rgb, vec3(2.2)) * specular.rgb;
//    vec3 mixedSpecular = mix(sampledSpecular, mixedAlbedo, vec3(metalness));
//
//    // Roughness
//    float roughness = 1.0 - smoothness;

    // Emissive color
    outColor = vec4(mixedAlbedo, 1.0);
}

