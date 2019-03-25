#version 330 core

uniform sampler2D colorTexture;

in vec2 texCoord;

out vec4 outColor;

void main(void)
{
    outColor = texture(colorTexture, texCoord);

    // Gamma correction
    outColor = pow(outColor, vec4(1.0/2.2));
    outColor.a = 1.0;
}
