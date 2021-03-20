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

layout(binding = 0) uniform Transforms
{
    mat4 uModelViewProjection;
};

out vec2 vTexCoord;
out vec3 vNormal;

void main()
{
    vTexCoord = aTexCoord;
	vNormal = aNormal;
    gl_Position = uModelViewProjection * vec4(aPosition, 1.0);
}

#elif defined(FRAGMENT) ///////////////////////////////////////////////

in vec2 vTexCoord;
in vec3 vNormal;

uniform sampler2D uTexture;

layout(location = 0) out vec4 oColor;

void main()
{
	vec4 albedo = texture(uTexture, vTexCoord);
	vec3 N = normalize(vNormal);
	vec3 L = normalize(vec3(1.0));
	float ambientFactor  = 0.2;
	float diffuseFactor  = 0.8 * max(0.0, dot(L,N));
    oColor = ambientFactor * albedo +
			 diffuseFactor * albedo;
}

#endif
#endif
