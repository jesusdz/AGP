#version 330 core

in vec3 localPosition;

uniform sampler2D equirectangularMap;

out vec4 outColor;

const vec2 invAtan = vec2(0.1591, 0.3183); // (1/2pi, 1/pi)
vec2 SampleSphericalMap(vec3 v)
{
    vec2 uv = vec2(atan(v.z, v.x), asin(v.y));
    uv *= invAtan;
    uv += 0.5;
    return uv;
}

void main()
{
    vec2 uv = SampleSphericalMap(normalize(localPosition)); // make sure to normalize localPos
    vec3 color = min(vec3(1000.0), texture(equirectangularMap, uv).rgb);

    outColor = vec4(color, 1.0);
}
