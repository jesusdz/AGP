///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
#ifdef SIMPLE_SHADER

#if defined(VERTEX) ///////////////////////////////////////////////////

layout(location=0) in vec3 aPosition;
layout(location=1) in vec2 aTexCoord;

out vec3 vColor;

void main()
{
    vColor = vec3(aTexCoord, 0.0);
    gl_Position = vec4(aPosition, 1.0);
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
#ifdef TEXTURED_GEOMETRY

#if defined(VERTEX) ///////////////////////////////////////////////////

layout(location=0) in vec3 aPosition;
layout(location=1) in vec2 aTexCoord;

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
#ifdef SHOW_MESH

#if defined(VERTEX) ///////////////////////////////////////////////////

layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aNormal;
//layout(location = 2) in vec2 aTexCoord;
//layout(location = 3) in vec3 aTangent;
//layout(location = 4) in vec3 aBitangent;

out vec3 vNormal;

void main()
{
    vNormal = aNormal;
    gl_Position = vec4(aPosition, 1.0) + vec4(0.0, 0.0, -0.5, 0.0);
    gl_Position.xyz *= vec3(0.2,0.2,-0.2);
}

#elif defined(FRAGMENT) ///////////////////////////////////////////////

in vec3 vNormal;

layout(location = 0) out vec4 oColor;

void main()
{
    oColor = vec4(vNormal, 1.0);
}

#endif
#endif



///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
#ifdef SHOW_TEXTURED_MESH

#if defined(VERTEX) ///////////////////////////////////////////////////

layout(location = 0) in vec3 aPosition;
//layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;
//layout(location = 3) in vec3 aTangent;
//layout(location = 4) in vec3 aBitangent;

out vec2 vTexCoord;

void main()
{
    vTexCoord = aTexCoord;
    gl_Position = vec4(aPosition, 1.0) + vec4(0.0, 0.0, -0.5, 0.0);
    gl_Position.xyz *= vec3(0.2,0.2,-0.2);
}

#elif defined(FRAGMENT) ///////////////////////////////////////////////

in vec2 vTexCoord;

uniform sampler2D uTexture;

layout(location = 0) out vec4 oColor;

void main()
{
    //oColor = vec4(vec2(vTexCoord), vec2(1.0));
    oColor = texture(uTexture, vTexCoord);
}

#endif
#endif



///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
#ifdef SHOW_TRANSFORMED_TEXTURED_MESH

#if defined(VERTEX) ///////////////////////////////////////////////////

layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;
//layout(location = 3) in vec3 aTangent;
//layout(location = 4) in vec3 aBitangent;

layout(binding = 0) uniform GlobalParams
{
    vec3 uCameraPosition;
};

layout(binding = 1) uniform LocalParams
{
    mat4 uWorldMatrix;
    mat4 uWorldViewProjectionMatrix;
};

out vec2 vTexCoord;
out vec3 vPosition; // In worldspace
out vec3 vNormal;   // In worldspace
out vec3 vViewDir;  // In worldspace

void main()
{
    vTexCoord = aTexCoord;
    vPosition = vec3( uWorldMatrix * vec4(aPosition, 1.0) );
	vNormal = vec3( uWorldMatrix * vec4(aNormal, 0.0) );
    vViewDir = uCameraPosition - vPosition;
    gl_Position = uWorldViewProjectionMatrix * vec4(aPosition, 1.0);
}

#elif defined(FRAGMENT) ///////////////////////////////////////////////

in vec2 vTexCoord;
in vec3 vPosition; // In worldspace
in vec3 vNormal;   // In worldspace
in vec3 vViewDir;  // In worldspace

uniform sampler2D uTexture;

layout(location = 0) out vec4 oColor;

void main()
{
	vec3 albedo = texture(uTexture, vTexCoord).rgb;
	vec3 N = normalize(vNormal);
	vec3 L = normalize(vec3(1.0));
    vec3 V = normalize(vViewDir);
    vec3 H = normalize(V + L);
	float ambientFactor  = 0.2;
	float diffuseFactor  = 0.7 * max(0.0, dot(L,N));
    float specularFactor = 0.3 * pow(max(0.0, dot(H,N)), 100.0);
    oColor = vec4(ambientFactor * albedo +
                  diffuseFactor * albedo +
                  specularFactor * vec3(1.0),
                  1.0);
}

#endif
#endif
