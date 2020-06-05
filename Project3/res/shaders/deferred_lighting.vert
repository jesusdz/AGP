#version 330 core

uniform mat4 worldMatrix;
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;

uniform int lightQuad;
uniform float lightRange;

layout (location=0) in vec3 position;

void main()
{
    if (lightQuad == 1) // Directional (plane)
    {
        gl_Position = vec4(position.xy, vec2(-1.0, 1.0));
    }
    else // Point light (sphere)
    {
        gl_Position = projectionMatrix * viewMatrix * worldMatrix * vec4(position * 8.0, 1.0);
    }
}
