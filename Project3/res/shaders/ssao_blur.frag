#version 330 core

uniform sampler2D ssaoMap;

in vec2 texCoords;

out vec4 outColor;

void main()
{
    vec2 texInc = 1.0 / textureSize(ssaoMap, 0);

    float blurred = 0.0;

    for (int i = -2; i < 2; ++i) {
        for (int j = -2; j < 2; ++j) {
            blurred += texture(ssaoMap, texCoords + texInc * vec2(j, i)).r;
        }
    }

    blurred /= 16.0;

    outColor = vec4(blurred);
}
