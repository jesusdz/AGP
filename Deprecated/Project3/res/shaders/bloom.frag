#version 330 core

uniform sampler2D colorMap;
uniform int maxLod;
uniform float lodIntensities[16];

in vec2 texCoord;

out vec4 outColor;

void main()
{
	outColor = vec4(0.0);
        for (int lod = 0; lod <= maxLod; ++lod)
        {
            //outColor += 0.03 * (float(lod+1) / float(maxLod+1)) * textureLod(colorMap, texCoord, float(lod));
            outColor += lodIntensities[lod] * textureLod(colorMap, texCoord, float(lod));
	}
	outColor.a = 1.0;
}

