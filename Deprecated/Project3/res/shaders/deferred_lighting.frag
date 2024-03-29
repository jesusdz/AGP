#version 330 core

// Camera
uniform float left;
uniform float right;
uniform float bottom;
uniform float top;
uniform float znear;
uniform float zfar;
uniform vec3 eyeWorldspace;
uniform mat4 viewMatrixInv;

// Viewport
uniform vec2 viewportSize;

// Light
uniform int lightType;
uniform vec3 lightPosition;
uniform vec3 lightDirection;
uniform vec3 lightColor;
uniform float lightRange;

// Environment
uniform samplerCube irradianceMap;
uniform samplerCube environmentMap;

// Textures
uniform sampler2D rt0; // albedo (rgb) + occlusion (a)
uniform sampler2D rt1; // specular (rgb) + roughness (a)
uniform sampler2D rt2; // normals(rgb) + unused (a)
uniform sampler2D rtD; // depth (r)

layout (location=0) out vec4 outColor;

// Returns the fragment position in world space
// Parameters
// d: depth (as taken from the depth buffer)
// l: left
// r: right
// b: bottom
// t: top
// n: znear
// f: zfar
// v: viewport size
vec3 reconstructPixelPosition(float d, float l, float r, float b, float t, float n, float f, vec2 v)
{
    float zndc = d * 2.0 - 1.0;
    float zeye = 2*f*n / (zndc*(f-n)-(f+n));// Converting from pixel coordinates to NDC
    float xndc = gl_FragCoord.x/v.x * 2.0 - 1.0;
    float yndc = gl_FragCoord.y/v.y * 2.0 - 1.0;
    float xeye = -zeye*(xndc*(r-l)+(r+l))/(2.0*n);
    float yeye = -zeye*(yndc*(t-b)+(t+b))/(2.0*n);
    vec3 eyecoords = vec3(xeye, yeye, zeye);
    return  vec3(viewMatrixInv * vec4(eyecoords, 1.0));
}

const float PI = 3.14159265359;

vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness)
{
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(1.0 - cosTheta, 5.0);
}

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


void main()
{
    vec2 uv = gl_FragCoord.xy / viewportSize;

    vec4 rtDpix = texture(rtD, uv);
    float depth = rtDpix.r;
    if (depth == 1.0) discard;

    vec3 fragmentWorlspace = reconstructPixelPosition(
                depth,
                left, right, bottom, top,
                znear, zfar,
                viewportSize);

    vec4 rt0pix = texture(rt0, uv);
    vec4 rt1pix = texture(rt1, uv);
    vec4 rt2pix = texture(rt2, uv);

    vec3 albedo = rt0pix.rgb;
    float occlusion = rt0pix.a;
    vec3 F0 = rt1pix.rgb; // specular
    float roughness = rt1pix.a;
    vec3 normal = normalize(rt2pix.rgb * 2.0 - vec3(1.0));

    vec3 N = normal;
    vec3 O = eyeWorldspace;
    vec3 P = fragmentWorlspace;
    vec3 V = normalize(O - P);

    if (lightType == 2) // Ambient light
    {
        // Ambient light is the sum of all indirect diffuse
        // and specular lights comming from all directions,
        // so we approximate it with the irradiance map
        vec3 kS = fresnelSchlickRoughness(max(dot(N, V), 0.0), F0, roughness);
        vec3 kD = vec3(1.0) - kS;

        vec3 irradiance = texture(irradianceMap, N).rgb;
        vec3 diffuse = kD * albedo * irradiance;
        outColor = vec4(diffuse, 1.0);

        // This trick should be done by pre-filtering the environment map
        // to construct a mip-map and accessing the proper level of detail
        vec3 reflectedRadiance = textureLod(environmentMap, reflect(-V, N), roughness * 15.0).rgb;
        vec3 specular = kS * reflectedRadiance;
        outColor += vec4(specular, 0.0);

        // Ambient occlusion
        outColor *= occlusion;
        return;
    }

    vec3 L = lightDirection;
    float attenuation = 1.0;
    if (lightType == 0) // Point light
    {
        L = lightPosition - P;
        float lightDistance = length(L);
        attenuation = 1.0/(lightDistance * lightDistance); // Kind of physically-based...
        attenuation *= smoothstep(0.0, lightRange, lightRange - lightDistance); // ... but adding a smooth transition towards its boundary :-/
    }
    L = normalize(L);
    vec3 H = normalize(L + V);
    float HdotV = max(dot(H, V), 0.0);
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);

    vec3 lightRadiance = lightColor * attenuation;

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

    outColor.rgb = (kD * albedo / PI + specular) * lightRadiance * NdotL;
}
