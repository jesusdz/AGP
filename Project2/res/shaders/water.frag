#version 330 core

uniform vec2 viewportSize;
uniform mat4 viewMatrixInv;
uniform mat4 projectionMatrixInv;
uniform sampler2D reflectionMap;
uniform sampler2D refractionMap;
uniform sampler2D depthMap;
uniform samplerCube environmentMap;

const float turbidityDistance = 10.0;

in Data
{
    vec3 positionViewspace;
    vec3 normalViewspace;
} FSIn;

out vec4 outColor;

vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

vec3 reconstructPixelPosition(float depth)
{
    vec2 texCoords = gl_FragCoord.xy / viewportSize;
    vec3 positionNDC = vec3(texCoords * 2.0 - vec2(1.0), depth * 2.0 - 1.0);
    vec4 positionEyespace = projectionMatrixInv * vec4(positionNDC, 1.0);
    positionEyespace.xyz /= positionEyespace.w;
    return positionEyespace.xyz;
}

void main()
{
    vec3 N = normalize(FSIn.normalViewspace);
    vec3 V = normalize(-FSIn.positionViewspace);
    vec3 F0 = vec3(0.03);
    vec3 F = fresnelSchlick(max(0.0, dot(V, N)), F0);
    vec2 texCoord = gl_FragCoord.xy / viewportSize;
    vec2 reflectionTexCoord = vec2(texCoord.s, 1.0 - texCoord.t);
    vec4 reflectionColor = vec4(texture(reflectionMap, reflectionTexCoord).rgb, 1.0);
    if (length(reflectionColor.rgb) < 0.01)
    {
    	vec3 R = reflect(-V, N);
	R = normalize(vec3(viewMatrixInv * vec4(R, 0.0)));
    	reflectionColor.rgb = texture(environmentMap, R).rgb;
    }
    float groundDepth = texture(depthMap, texCoord).x;
    vec3 groundPosViewspace = reconstructPixelPosition(groundDepth);
    float waterDepth = FSIn.positionViewspace.z - groundPosViewspace.z;
    vec4 waterColor = vec4(0.0, 0.5, 1.0, min(waterDepth / turbidityDistance, 1.0));
    outColor = mix(waterColor, reflectionColor, vec4(F, F.r));
}

