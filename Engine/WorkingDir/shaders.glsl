
struct Light
{
    uint type;
    vec3 color;
    vec3 direction;
    vec3 position;
};

#if VERSION > 410
#   define UNIFORM_BLOCK(bindingNumber) layout(binding = bindingNumber, std140)
#else
#   define UNIFORM_BLOCK(bindingNumber) layout(std140)
#endif

///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
#ifdef TEXTURED_GEOMETRY

#if defined(VERTEX) ///////////////////////////////////////////////////

layout(location=0) in vec3 aPosition;
layout(location=2) in vec2 aTexCoord;

out vec2 vTexCoord;

void main()
{
    vTexCoord = aTexCoord;
    gl_Position = vec4(aPosition, 1.0);
}

#elif defined(FRAGMENT) ///////////////////////////////////////////////

in vec2 vTexCoord;

uniform sampler2D uTexture;

layout(location = 0) out vec4 oColor;

void main()
{
    oColor = texture(uTexture, vTexCoord);
}

#endif
#endif



///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
#ifdef DEBUG_DRAW_OPAQUE

#if defined(VERTEX) ///////////////////////////////////////////////////

layout(location = 0) in vec3 aPosition;
layout(location = 5) in vec3 aColor;

UNIFORM_BLOCK(0) uniform GlobalParams
{
    mat4  uViewProjectionMatrix;
    vec3  uCameraPosition;
    uint  uLightCount;
    Light uLight[16];
};

out vec3 vColor;

void main()
{
    vColor = aColor;
    gl_Position = uViewProjectionMatrix * vec4(aPosition, 1.0);
}

#elif defined(FRAGMENT) ///////////////////////////////////////////////

in vec3 vColor;

layout(location = 0) out vec4 oColor;

void main()
{
    oColor = vec4(vColor, 1.0);
}

#endif
#endif



///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
#ifdef FORWARD_RENDER

#if defined(VERTEX) ///////////////////////////////////////////////////

#define USE_INSTANCING

layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;
//layout(location = 3) in vec3 aTangent;
//layout(location = 4) in vec3 aBitangent;

#if defined(USE_INSTANCING)
layout(location = 6) in mat4 aWorldMatrix;
layout(location = 10) in mat4 aWorldViewProjectionMatrix;
#endif

UNIFORM_BLOCK(0) uniform GlobalParams
{
    mat4  uViewProjectionMatrix;
    vec3  uCameraPosition;
    uint  uLightCount;
    Light uLight[16];
};

#if !defined(USE_INSTANCING)
UNIFORM_BLOCK(1) uniform LocalParams
{
    mat4 aWorldMatrix;
    mat4 aWorldViewProjectionMatrix;
};
#endif

out vec2 vTexCoord;
out vec3 vPosition; // In worldspace
out vec3 vNormal;   // In worldspace
out vec3 vViewDir;  // In worldspace

void main()
{
    vTexCoord = aTexCoord;
    vPosition = vec3( aWorldMatrix * vec4(aPosition, 1.0) );
    vNormal = vec3( aWorldMatrix * vec4(aNormal, 0.0) );
    vViewDir = uCameraPosition - vPosition;
    gl_Position = aWorldViewProjectionMatrix * vec4(aPosition, 1.0);
}

#elif defined(FRAGMENT) ///////////////////////////////////////////////

in vec2 vTexCoord;
in vec3 vPosition; // In worldspace
in vec3 vNormal;   // In worldspace
in vec3 vViewDir;  // In worldspace

uniform sampler2D uAlbedo;

UNIFORM_BLOCK(0)  uniform GlobalParams
{
    mat4  uViewProjectionMatrix;
    vec3  uCameraPosition;
    uint  uLightCount;
    Light uLight[16];
};

layout(location = 0) out vec4 oColor;

void main()
{
    vec3 albedo = texture(uAlbedo, vTexCoord).rgb;
    vec3 N = normalize(vNormal);
    vec3 V = normalize(vViewDir);

    float ambientFactor = 0.05;
    oColor = vec4(ambientFactor * albedo, 1.0);

    for (uint i = 0; i < uLightCount; ++i)
    {
        vec3 L = uLight[i].direction;
        if (uLight[i].type == 1) L = normalize(uLight[i].position - vPosition);

        vec3 H = normalize(V + L);

        float attenuationFactor = 1.0;
        if (uLight[i].type == 1) attenuationFactor = 1.0 / length(uLight[i].position - vPosition);

        float diffuseFactor  = 0.7 * max(0.0, dot(L,N));
        oColor.rgb += diffuseFactor * uLight[i].color * attenuationFactor * albedo;

        float specularFactor = 0.3 * pow(max(0.0, dot(H,N)), 100.0);
        oColor.rgb += specularFactor * uLight[i].color * attenuationFactor;
    }
}

#endif
#endif
