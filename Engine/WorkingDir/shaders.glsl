///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
#ifdef SIMPLE_SHADER

///////////////////////////////////////////////////////////////////////
#if defined(VERTEX)

layout(location=0) in vec3 aPosition;
layout(location=1) in vec2 aTexCoord;

out vec3 vColor;

void main()
{
    vColor = vec3(aTexCoord, 0.0);
    gl_Position = vec4(aPosition, 1.0);
}

///////////////////////////////////////////////////////////////////////
#elif defined(FRAGMENT)

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

///////////////////////////////////////////////////////////////////////
#if defined(VERTEX)

layout(location=0) in vec3 aPosition;
layout(location=1) in vec2 aTexCoord;

out vec2 vTexCoord;

void main()
{
    vTexCoord = aTexCoord;
    gl_Position = vec4(aPosition, 1.0);
}

///////////////////////////////////////////////////////////////////////
#elif defined(FRAGMENT)

in vec2 vTexCoord;

uniform sampler2D uTexture;

layout(location = 0) out vec4 oColor;

void main()
{
    oColor = texture(uTexture, vTexCoord);
}

#endif
#endif
