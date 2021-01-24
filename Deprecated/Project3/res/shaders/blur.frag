#version 330 core

uniform sampler2D colorMap;
uniform vec2 direction;
uniform int inputLod;
uniform float radius;

in vec2 texCoords;

out vec4 outColor;

void main()
{
	vec2 texSize = textureSize(colorMap, inputLod);
	vec2 texelSize = 1.0/texSize;
	vec2 margin1 = texelSize * 0.5;
	vec2 margin2 = vec2(1.0) - margin1;
	
	outColor = vec4(0.0);
	
        vec2 directionFragCoord = gl_FragCoord.xy * direction;
	int coord = int(directionFragCoord.x + directionFragCoord.y);
        vec2 directionTexSize = texSize * direction;
	int size = int(directionTexSize.x + directionTexSize.y);
        int kernelRadius = int(radius);
        int kernelBegin = -min(kernelRadius, coord);
        int kernelEnd = min(kernelRadius, size - coord);
	float weight = 0.0;
	for (int i = kernelBegin; i <= kernelEnd; ++i)
	{
                //float currentWeight = smoothstep(float(kernelRadius), 0.0, float(abs(i)));
                float currentWeight = pow(1.0 - float(abs(i))/float(kernelRadius), 2.0);
		vec2 finalTexCoords = texCoords + i * direction * texelSize;
		finalTexCoords = clamp(finalTexCoords, margin1, margin2);
		outColor += textureLod(colorMap, finalTexCoords, inputLod) * currentWeight;
		weight += currentWeight;
	}

        outColor = outColor / weight;
}

