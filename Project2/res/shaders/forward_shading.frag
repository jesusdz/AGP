#version 330 core

// Matrices
uniform mat4 worldViewMatrix;
uniform mat3 normalMatrix;

// Material
uniform vec4 albedo;
uniform vec4 specular;
uniform vec4 emissive;
uniform float smoothness;
uniform float bumpiness;
uniform vec2 tiling;
uniform sampler2D albedoTexture;
uniform sampler2D specularTexture;
uniform sampler2D emissiveTexture;
uniform sampler2D normalTexture;
uniform sampler2D bumpTexture;

// Lights
#define MAX_LIGHTS 8
uniform int lightType[MAX_LIGHTS];
uniform vec3 lightPosition[MAX_LIGHTS];
uniform vec3 lightDirection[MAX_LIGHTS];
uniform vec3 lightColor[MAX_LIGHTS];
uniform int lightCount;

in Data
{
    vec3 positionViewspace;
    vec3 normalLocalspace;
    vec2 texCoords;
    vec3 tangentLocalspace;
    vec3 bitangentLocalspace;
} FSIn;

out vec4 outColor;


const float PI = 3.14159265359;

#define COOK_TORRANCE
//#define BLINN_PHONG


#ifdef COOK_TORRANCE

vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a      = roughness*roughness;
    float a2     = a*a;
    float NdotH  = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;

    float num   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return num / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float num   = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return num / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2  = GeometrySchlickGGX(NdotV, roughness);
    float ggx1  = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}

// Parallax occlusion mapping aka. relief mapping
vec2 reliefMapping(vec2 texCoords)
{
    // Compute the ray in texture space
    vec3 T = normalize(FSIn.tangentLocalspace);
    vec3 B = normalize(FSIn.bitangentLocalspace);
    vec3 N = normalize(FSIn.normalLocalspace);
    mat3 TBNInverse = transpose(mat3(T, -B, N));
    mat4 worldViewMatrixInverse = inverse(worldViewMatrix);
    vec3 rayEyespace = normalize(FSIn.positionViewspace);
    vec3 rayTexspace = TBNInverse * mat3(worldViewMatrixInverse) * rayEyespace;

    // Increment
    float heightScale = bumpiness;
    vec3 rayIncrementTexspace;
    float texSize = max(textureSize(bumpTexture, 0).x, textureSize(bumpTexture, 0).y) * 2.0;
    rayIncrementTexspace.xy = rayTexspace.xy / abs(rayTexspace.z * texSize);
    rayIncrementTexspace.z = 1.0/bumpiness;

    // Sampling state
    vec3 samplePositionTexspace = vec3(texCoords, 0.0);
    float sampledDepth = 1.0;

    // Linear search
    for (int i = 0; i < 10 && samplePositionTexspace.z < sampledDepth; ++i)
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
    float fragDist = length(FSIn.positionViewspace);
    vec3 V = - FSIn.positionViewspace / fragDist;

    outColor.rgb = vec3(0.0);

    vec2 texCoords = FSIn.texCoords * tiling;

#define USE_RELIEF_MAPPING
#ifdef USE_RELIEF_MAPPING
    if (bumpiness > 0.0) {
        texCoords = reliefMapping(texCoords);
    }
#endif

    vec4 sampledAlbedo = pow(texture(albedoTexture, texCoords), vec4(2.2));
    if (sampledAlbedo.a < 0.01) { discard; }
    vec3 mixedAlbedo = albedo.rgb * sampledAlbedo.rgb;

    vec3 sampledSpecular = pow(texture(specularTexture, texCoords).rgb, vec3(2.2));
    vec3 mixedSpecular = specular.rgb * sampledSpecular;

#define USE_NORMAL_MAPPING
#ifdef USE_NORMAL_MAPPING
    // Normal in viewspace
    vec3 viewspaceNormal = normalize(normalMatrix * FSIn.normalLocalspace);

    // Tangent to local (TBN) matrix
    vec3 T = normalize(FSIn.tangentLocalspace);
    vec3 B = normalize(FSIn.bitangentLocalspace);
    vec3 N = normalize(FSIn.normalLocalspace);
    // Gram-Schmidth
//    T = normalize(T - N * dot(T, N));
//    B = normalize(B - N * dot(B, N) - T * dot(B, T));
    mat3 TBN = mat3(T, B, N);

    // Modified normal in viewspace
    vec3 tangentSpaceNormal = texture(normalTexture, texCoords).xyz * 2.0 - vec3(1.0);
    //tangentSpaceNormal.y *= 1.0/0.2;
    vec3 localSpaceNormal = TBN * tangentSpaceNormal;
    vec3 modifiedViewSpaceNormal = normalize(mat3(worldViewMatrix) * localSpaceNormal);

    N = mix(
        viewspaceNormal,
        modifiedViewSpaceNormal,
        length(T) > 0.001);
#else
    // Normal without modifying in viewspace
    vec3 N = normalize(normalMatrix * FSIn.normalLocalspace, 0.0);
#endif

    // ambient light
    float ambientTerm = 0.05;
    outColor.rgb += mixedAlbedo * ambientTerm;

    // Caracteristic specular color (fresnel at 0 degrees)
    vec3 F0 = vec3(0.04);
    //F0 = mix(F0, albedo, metallic);

    float radius = 30.0;

    float roughness = 1.0 - smoothness;

    for (int i = 0; i < lightCount; ++i)
    {
        // For directional lights
        vec3 L = lightDirection[i];
        vec3 radiance = lightColor[i];

        // For point lights
        float attenuationFactor = 1.0;
        if (lightType[i] == 0) {
            L = lightPosition[i] - FSIn.positionViewspace;
            float distance = length(L);
            L = L / distance;
            attenuationFactor = clamp(1.0 - (distance * distance)/ (radius * radius), 0.0, 1.0);
            radiance *= attenuationFactor;
        }
        vec3 H = normalize(L + V);
        float NdotL = max(dot(N, L), 0.0);
        float HdotV = max(dot(H, V), 0.0);
        float NdotV = max(dot(N, V), 0.0);

        // cook-torrance brdf
        float NDF = DistributionGGX(N, H, roughness);
        float G   = GeometrySmith(N, V, L, roughness);
        vec3 F    = fresnelSchlick(HdotV, F0);

        vec3 kS = F;
        vec3 kD = vec3(1.0) - kS;
        //kD *= 1.0 - metallic;

        vec3 numerator    = NDF * G * F;
        float denominator = 4.0 * NdotV * NdotL;
        vec3 specular     = numerator / max(denominator, 0.001);

        outColor.rgb += (kD * mixedAlbedo / PI + specular) * radiance * NdotL;
    }

    outColor.a = 1.0;

    // Emissive color
    outColor.rgb += emissive.rgb;

    // Gamma correction
    outColor.rgb = pow(outColor.rgb, vec3(1.0/2.2));
}

#endif

#ifdef BLINN_PHONG

// Blinn-Phong reflection model
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
    vec3 T = normalize(FSIn.tangentLocalspace);
    vec3 B = normalize(FSIn.bitangentLocalspace);
    vec3 N = normalize(FSIn.normalLocalspace);
    mat3 TBN = mat3(T, B, N);

    // Modified normal in viewspace
    vec3 tangentSpaceNormal = texture(normalTexture, FSIn.texCoords).xyz * 2.0 - vec3(1.0);
    vec3 localSpaceNormal = TBN * tangentSpaceNormal;
    vec3 viewSpaceNormal = normalMatrix * localSpaceNormal;

    N = mix(
        normalize(normalMatrix * FSIn.normalLocalspace),
        viewSpaceNormal,
        (float)length(T) > 0.001);
#else
    // Normal without modifying in viewspace
    vec3 N = normalize(normalMatrix * FSIn.normalLocalspace);
#endif

    float ambientTerm = 0.05;
    outColor.rgb += mixedAlbedo * ambientTerm;

    float radius = 30.0;

    for (int i = 0; i < lightCount; ++i)
    {
        vec3 L = lightPosition[i] - FSIn.positionViewspace;
        float distance = length(L);
        L = normalize(L);
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

        vec3 lightContribution = lightColor[i] * (mixedAlbedo.rgb * kD + mixedSpecular.rgb * kS);

        // Attenuation
        float attenuationFactor = clamp(1.0 - (distance * distance)/ (radius * radius), 0.0, 1.0);
        lightContribution *= attenuationFactor;

        outColor.rgb += lightContribution;
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
#endif
