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

// Textures
uniform sampler2D rt0; // albedo (rgb) + occlusion (a)
uniform sampler2D rt1; // specular (rgb) + roughness (a)
uniform sampler2D rt2; // normals(rgb) + unused (a)
uniform sampler2D rt4; // depth (r)

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

void main()
{
    vec2 uv = gl_FragCoord.xy / viewportSize;

    vec4 rt4pix = texture(rt4, uv);
    float depth = rt4pix.r;
    if (depth > 0.9999) discard;

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
    vec3 specular = rt1pix.rgb;
    float roughness = rt1pix.a;
    vec3 normal = normalize(rt2pix.rgb * 2.0 - vec3(1.0));

    vec3 N = normal;
    vec3 O = eyeWorldspace;
    vec3 P = fragmentWorlspace;
    vec3 V = normalize(O - P);

    vec3 L = lightDirection;
    float attenuation = 1.0;
    if (lightType == 0) // Point light
    {
        L = lightPosition - P;
        float lightDistance = length(L);
        attenuation = 1.0/(lightDistance * lightDistance);
        attenuation *= smoothstep(0.0, lightRange, lightRange - lightDistance);
    }
    L = normalize(L);
    vec3 H = normalize(L + V);

    vec3 lightRadiance = lightColor * attenuation;

    float NdotL = max(0.0, dot(N, L));
    float NdotH = max(0.0, dot(N, H));

    float lambert = NdotL;
    float phong = pow(NdotH, max(1.0, 255.0 * (1.0 - roughness)));

    vec3 ambientRadiance = 0.0 * vec3(occlusion);

    vec3 diffuseRadiance = lightRadiance * albedo * lambert;

    vec3 specularRadiance = lightRadiance * phong;

    vec3 surfRadiance = ambientRadiance + diffuseRadiance + specularRadiance;
    outColor = vec4(surfRadiance, 1.0);
}
