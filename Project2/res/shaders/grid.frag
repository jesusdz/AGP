#version 330 core

uniform float left;
uniform float right;
uniform float bottom;
uniform float top;
uniform float znear;

in vec2 texCoord;
out vec4 outColor;

void main()
{
    vec3 bottomLeftCorner = vec3(left, bottom, -znear);
    vec3 topRightCorner = vec3(right, top, -znear);
    vec3 mixFactors = vec3(texCoord, 0.0);
    vec3 eyeVector = mix(bottomLeftCorner, topRightCorner, mixFactors);
    outColor = vec4(1.0, 0.0, 0.0, 1.0);
}
