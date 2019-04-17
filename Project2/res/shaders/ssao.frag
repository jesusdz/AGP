#version 330 core

// Viewport
uniform vec2 viewportSize;

// Camera
uniform float left;
uniform float right;
uniform float bottom;
uniform float top;
uniform float znear;
uniform float zfar;
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;

// Textures
uniform sampler2D normalMap;
uniform sampler2D depthMap;

// SSAO params
uniform sampler2D noiseMap;
uniform vec3 samples[64];


in vec2 texCoords;

out vec4 outColor;


// Returns the fragment position in view space
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
    return  eyecoords;
}

void main()
{
    float depth = texture(depthMap, texCoords).r;

    vec2 noiseScale = viewportSize / textureSize(noiseMap, 0);

    vec3 fragPosView = reconstructPixelPosition(depth, left, right, bottom, top, znear, zfar, viewportSize);

    vec3 normalWorld = texture(normalMap, texCoords).rgb;
    vec3 normalView = vec3(viewMatrix * vec4(normalWorld, 0.0));

    vec3 randomVec = texture(noiseMap, texCoords * noiseScale).xyz;

    // Create tangent to world basis
    vec3 tangent   = normalize(randomVec - normalView * dot(randomVec, normalView));
    vec3 bitangent = cross(normalView, tangent);
    mat3 TBN       = mat3(tangent, bitangent, normalView);

    float radius = 1.0;

    float occlusion = 0.0;

    for(int i = 0; i < 64; ++i)
    {
        // get sample position
        vec3 sample = TBN * samples[i]; // From tangent to view-space
        sample = fragPosView + sample * radius;

        vec4 offset = vec4(sample, 1.0);
        offset      = projectionMatrix * offset; // from view to clip-space
        offset.xyz /= offset.w;                  // perspective divide
        offset.xyz  = offset.xyz * 0.5 + 0.5;    // transform to range 0.0 - 1.0

        float sampledDepth = texture(depthMap, offset.xy).r;
        vec3 sampledPosView = reconstructPixelPosition(sampledDepth, left, right, bottom, top, znear, zfar, viewportSize);

        float rangeCheck = smoothstep(0.0, 1.0, radius / abs(sample.z - sampledPosView.z));
        rangeCheck *= rangeCheck;

        occlusion += (sample.z <= sampledPosView.z - 0.1 ? 1.0 : 0.0) * rangeCheck;
    }

    outColor = vec4(1.0 - occlusion / 64.0);
}
