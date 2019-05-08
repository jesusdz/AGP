#version 330 core

uniform sampler2D colorMap;
uniform vec2 direction;
uniform int inputLod;

in vec2 texCoords;

out vec4 outColor;

void main()
{
	vec2 texelSize = 1.0/textureSize(colorMap, inputLod);
    outColor =
		textureLod(colorMap, texCoords - 2.0 * direction * texelSize, inputLod) +
		textureLod(colorMap, texCoords - 1.0 * direction * texelSize, inputLod) +
		textureLod(colorMap, texCoords + 0.0 * direction * texelSize, inputLod) +
		textureLod(colorMap, texCoords + 1.0 * direction * texelSize, inputLod) +
		textureLod(colorMap, texCoords + 2.0 * direction * texelSize, inputLod);
	outColor /= 5.0;
}

