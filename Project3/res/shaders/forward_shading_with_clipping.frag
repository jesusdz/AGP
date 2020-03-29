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

// Environment lighting
uniform samplerCube irradianceMap;

in Data
{
    vec3 positionWorldspace;
    vec3 normalWorldspace;
    vec2 texCoords;
} FSIn;

layout(location=0) out vec4 outColor;



void main(void)
{
    vec2 texCoords = FSIn.texCoords;

    // irradiance
    vec3 lightIrradiance = texture(irradianceMap, FSIn.normalWorldspace).rgb;

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
    outColor = vec4(mixedAlbedo * lightIrradiance, 1.0);
}

