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
uniform int lightType[MAX_LIGHTS];
uniform vec3 lightPosition[MAX_LIGHTS];
uniform vec3 lightDirection[MAX_LIGHTS];
uniform vec3 lightColor[MAX_LIGHTS];
uniform int lightCount;

in Data
{
    vec3 positionViewspace;
    vec3 normalLocalspace;
} FSIn;

out vec4 outColor;


const float PI = 3.14159265359;

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

void main(void)
{
    float fragDist = length(FSIn.positionViewspace);
    vec3 V = - FSIn.positionViewspace / fragDist;

    outColor.rgb = vec3(0.0);

    // Normal without modifying in viewspace
    vec3 N = normalize(worldViewMatrix * vec4(FSIn.normalLocalspace, 0.0)).xyz;

    // ambient light
    float ambientTerm = 0.05;
    outColor.rgb += albedo.rgb * ambientTerm;

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

        outColor.rgb += (kD * albedo.rgb / PI + specular) * radiance * NdotL;
    }

    outColor.a = 1.0;

    // Emissive color
    outColor.rgb += emissive.rgb;

    // Gamma correction
    outColor.rgb = pow(outColor.rgb, vec3(1.0/2.2));
}
