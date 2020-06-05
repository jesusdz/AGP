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

    if (depth == 1.0) {
        outColor = vec4(1.0);
        return;
    }


    vec3 fragPosView = reconstructPixelPosition(depth, left, right, bottom, top, znear, zfar, viewportSize);

    vec3 normalWorld = texture(normalMap, texCoords).rgb * 2.0 - vec3(1.0);
    vec3 normalView = vec3(viewMatrix * vec4(normalWorld, 0.0));

    // random rotation vector
    vec2 noiseScale = viewportSize / textureSize(noiseMap, 0);
    vec3 randomVec = texture(noiseMap, texCoords * noiseScale).xyz;

    // tangent to view basis
    vec3 tangent   = normalize(randomVec - normalView * dot(randomVec, normalView));
    vec3 bitangent = cross(normalView, tangent);
    mat3 TBN       = mat3(tangent, bitangent, normalView);

    float radius = 2.0;

    float occlusion = 0.0;

    for(int i = 0; i < 64; ++i)
    {
        // get neighboring sample position
        vec3 offsetView = TBN * samples[i];                     // tangent to view-space
        vec3 samplePosView = fragPosView + offsetView * radius; // offset from fragment pos

        // project to texture coordinates
        vec4 sampleTexCoord = projectionMatrix * vec4(samplePosView, 1.0); // from view to clip-space
        sampleTexCoord.xyz /= sampleTexCoord.w;                            // perspective divide
        sampleTexCoord.xyz  = sampleTexCoord.xyz * 0.5 + 0.5;              // transform to range 0.0 - 1.0

        // texture look-up and position reconstruction
        float sampledDepth = texture(depthMap, sampleTexCoord.xy).r;
        vec3 sampledPosView = reconstructPixelPosition(sampledDepth, left, right, bottom, top, znear, zfar, viewportSize);

        // range check (distant foreground pixels don't occlude)
        float rangeCheck = smoothstep(0.0, 1.0, radius / abs(samplePosView.z - sampledPosView.z));
        rangeCheck *= rangeCheck;

        // sum occlusion
        occlusion += (samplePosView.z < sampledPosView.z - 0.02 ? 1.0 : 0.0) * rangeCheck;
    }

    outColor = vec4(1.0 - occlusion / 64.0);
}
