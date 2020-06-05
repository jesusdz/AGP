#version 330 core

uniform sampler2D fontTexture;

in vec2 vTexCoord;

out vec4 outColor;

void main()
{
    float alpha = texture(fontTexture, vTexCoord).r;
    outColor = vec4(vec3(1.0), alpha);
}
