#version 330 core

uniform sampler2D colorTexture;
uniform bool blitAlpha;

in vec2 texCoord;

out vec4 outColor;

void main(void)
{
    vec4 texel = texture(colorTexture, texCoord);

    if (blitAlpha) {
        outColor.rgb = vec3(texel.a);
    } else {
        outColor.rgb = texel.rgb;
    }

    // Gamma correction
    outColor = pow(outColor, vec4(1.0/2.2));
    outColor.a = 1.0;
}
