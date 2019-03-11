#version 330 core

// Matrices
uniform mat4 worldViewMatrix;

// Material
uniform vec4 albedo;
uniform vec4 specular;
uniform vec4 emissive;
uniform float smoothness;
uniform sampler2D albedoTexture;
uniform sampler2D specularTexture;
uniform sampler2D emissiveTexture;
uniform sampler2D normalTexture;
uniform sampler2D bumpTexture;

// Lights
#define MAX_LIGHTS 8
uniform vec3 lightPosition[MAX_LIGHTS];
uniform vec3 lightColor[MAX_LIGHTS];
uniform int lightCount;

in Data
{
    vec3 positionViewspace;
    vec3 normalLocalspace;
    vec2 texCoords;
    vec3 tangent;
    vec3 bitangent;
} FSIn;

out vec4 outColor;

void main(void)
{
    float fragDist = length(FSIn.positionViewspace);
    vec3 V = - FSIn.positionViewspace / fragDist;

    outColor.rgb = vec3(0.0);

    vec4 sampledAlbedo = pow(texture(albedoTexture, FSIn.texCoords), vec4(2.2));
    if (sampledAlbedo.a < 0.01) { discard; }
    vec3 mixedAlbedo = albedo.rgb * sampledAlbedo.rgb;

    vec3 sampledSpecular = pow(texture(specularTexture, FSIn.texCoords).rgb, vec3(2.2));
    vec3 mixedSpecular = specular.rgb * sampledSpecular;

#define USE_NORMAL_MAPPING
#ifdef USE_NORMAL_MAPPING
    // Tangent to local (TBN) matrix
    vec3 T = normalize(FSIn.tangent);
    vec3 B = normalize(FSIn.bitangent);
    vec3 N = normalize(FSIn.normalLocalspace);
    mat3 TBN = mat3(T, B, N);

    // Modified normal in viewspace
    vec3 tangentSpaceNormal = texture(normalTexture, FSIn.texCoords).xyz * 2.0 - vec3(1.0);
    vec3 localSpaceNormal = TBN * tangentSpaceNormal;
    N = normalize(worldViewMatrix * vec4(localSpaceNormal, 0.0)).xyz;
#else
    // Normal without modifying in viewspace
    vec3 N = normalize(worldViewMatrix * vec4(FSIn.normalLocalspace, 0.0)).xyz;
#endif

    float ambientTerm = 0.05;
    outColor.rgb += mixedAlbedo * ambientTerm;

    for (int i = 0; i < lightCount; ++i)
    {
        vec3 L = normalize(lightPosition[i] - FSIn.positionViewspace);
        float kD = max(0.0, dot(L, N));

#define BLINN_PHONG
#ifdef BLINN_PHONG
        vec3 H = normalize(L + V);
        float kS = pow(max(0.0, dot(H, N)), 1.0 + smoothness * 255.0);
#else
        vec3 R = reflect(-L, N);
        float kS = pow(max(0.0, dot(R, V)), 1.0 + smoothness * 255.0);
#endif

        kS *= 0.1 + 0.9 * smoothness; // Reduce intensity as the shininess gets broader
        kS *= step(0.001, kD);        // Cancel specularity if LdotN is less than 0

        outColor.rgb += lightColor[i] * (mixedAlbedo.rgb * kD + mixedSpecular.rgb * kS);
    }

    outColor.a = 1.0;

    // Fog
//    vec3 fogColor = vec3(0.0, 0.0, 0.0);
//    outColor.rgb = mix(outColor.rgb, fogColor, length(FSIn.positionViewspace)/50.0);

    // Emissive color
    outColor.rgb += emissive.rgb;

    // Gamma correction
    outColor.rgb = pow(outColor.rgb, vec3(1.0/2.4));
}

