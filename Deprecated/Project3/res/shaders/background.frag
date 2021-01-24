#version 330 core

uniform vec2 viewportSize;
uniform float left;
uniform float right;
uniform float bottom;
uniform float top;
uniform float znear;
uniform mat4 worldMatrix;

uniform samplerCube environmentMap;
uniform samplerCube irradianceMap;

out vec4 outColor;

//const vec2 normalizationFactor = vec2(0.1591, 0.3183); // (1 / 2pi, 1 / pi)
//vec2 sphericalCoordsFromDirection(vec3 v)
//{
//    vec2 uv = vec2(atan(v.z, v.x), asin(v.y));
//    uv = uv * normalizationFactor + vec2(0.5);
//    return uv;
//}

void main()
{
    // Angle-based viewing ray elevation in world space
    vec2 texCoords = gl_FragCoord.xy / viewportSize;
    vec3 rayViewspace = normalize(vec3(vec2(left, bottom) + texCoords * vec2(right-left, top-bottom), -znear));
    vec3 rayWorldspace = vec3(worldMatrix * vec4(rayViewspace, 0.0));

	outColor.rgb = texture(environmentMap, normalize(rayWorldspace)).rgb;
	outColor.a = 1.0;
}
