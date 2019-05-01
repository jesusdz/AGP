#version 330 core

uniform vec2 viewportSize;
uniform mat4 modelViewMatrix;
uniform mat4 viewMatrixInv;
uniform mat4 projectionMatrixInv;
uniform sampler2D reflectionMap;
uniform sampler2D refractionMap;
uniform sampler2D depthMap;
uniform sampler2D normalMap;
uniform sampler2D dudvMap;

const float turbidityDistance = 10.0;

const int NUM_WAVES = 2;

in Data
{
    vec3 positionViewspace;
    vec3 normalViewspace;
} FSIn;

out vec4 outColor;

vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

vec3 reconstructPixelPosition(float depth)
{
    vec2 texCoords = gl_FragCoord.xy / viewportSize;
    vec3 positionNDC = vec3(texCoords * 2.0 - vec2(1.0), depth * 2.0 - 1.0);
    vec4 positionEyespace = projectionMatrixInv * vec4(positionNDC, 1.0);
    positionEyespace.xyz /= positionEyespace.w;
    return positionEyespace.xyz;
}

void main()
{
#if 0
    // position of the fragment in worldspace
		vec3  P = vec3(viewMatrixInv * vec4(FSIn.positionViewspace, 1.0));
    // stepness
    float Q[NUM_WAVES] = float[](0.0, 0.0); 
    // frequency ( frequency = 2 * pi / wavelength )
    float w[NUM_WAVES] = float[](5.5, 5.5);
    // amplitude
    float A[NUM_WAVES] = float[](0.1, 0.1);
    // phase ( phase = speed * 2 * pi / wavelength )
    float y[NUM_WAVES] = float[](0.0, 0.0);
    // direction
    vec3  D[NUM_WAVES] = vec3[](
				normalize(P - vec3(20.0, 0.0, 20.0)),
				normalize(P - vec3(20.0, 0.0, 9.0)));
		float t = 0.0;

		float sumBx = 0.0;
		float sumBy = 0.0;
		float sumBz = 0.0;
		float sumTx = 0.0;
		float sumTy = 0.0;
		float sumTz = 0.0;
		float sumNx = 0.0;
		float sumNy = 0.0;
		float sumNz = 0.0;
		for (int i = 0; i < 1; ++i)
		{
			float WA = w[i] * A[i];
			float S = sin(w[i] * dot(D[i], P) + y[i] * t);
			float C = cos(w[i] * dot(D[i], P) + y[i] * t);

			sumBx += Q[i] * D[i].x * D[i].x * WA * S;
			sumBy += Q[i] * D[i].x * D[i].y * WA * S;
			sumBz += D[i].x * WA * C;
			sumTx += Q[i] * D[i].x * D[i].y * WA * S;
			sumTy += Q[i] * D[i].y * D[i].y * WA * S;
			sumTz += D[i].y * WA * C;
			sumNx += D[i].x * WA * C;
			sumNy += D[i].y * WA * C;
			sumNz += Q[i] * WA * S;
		}

//		vec3 sumT = normalize(vec3(sumTx, sumTy, sumTz));
//		vec3 sumB = normalize(vec3(sumBx, sumBy, sumBz));
//		vec3 sumN = normalize(vec3(sumNx, sumNy, sumNz));
//		vec3 B  = vec3(1.0 - sumB.x, - sumB.y, sumB.z);
//		vec3 T  = vec3(-sumT.x, 1.0 - sumT.y, sumT.z);
//		vec3 Nw = vec3(-sumN.x, -sumN.y, 1.0 - sumN.z);
		vec3 B  = vec3(1.0 - sumBx, - sumBy, sumBz);
		vec3 T  = vec3(-sumTx, 1.0 - sumTy, sumTz);
		vec3 Nw = vec3(-sumNx, -sumNy, 1.0 - sumNz);

		outColor.rgb = 0.5 * Nw + vec3(0.5);
		outColor.a = 1.0;
		return;
#endif

    vec3 N = normalize(FSIn.normalViewspace);
    vec3 V = normalize(-FSIn.positionViewspace);
    vec2 texCoord = gl_FragCoord.xy / viewportSize;
    float groundDepth = texture(depthMap, texCoord).x;

    vec3 Pw = vec3(viewMatrixInv * vec4(FSIn.positionViewspace, 1.0));
    vec2 waveLength = vec2(5.0);
    vec2 waveStrength = vec2(0.04);
    vec3 Ntangent = vec3(2.0) * texture(normalMap, Pw.xz * 0.5).rgb - vec3(1.0);
    //vec3 N = vec3(modelViewMatrix * vec4(Ntangent.x, Ntangent.z, Ntangent.y, 0.0)); // viewspace
    vec2 DUV = vec2(2.0) * texture(dudvMap, Pw.xz / waveLength).rg - vec2(1.0);
    vec2 distortion = DUV * waveStrength;
    distortion *= dot(N,V);
//		//outColor.rgb = Ntangent;
//		outColor.rgb = Ntangent;
//		outColor.b = 0.0;
//		outColor.a = 1.0;
//		return;

    vec3 groundPosViewspace = reconstructPixelPosition(groundDepth);
    float waterDepth = FSIn.positionViewspace.z - groundPosViewspace.z;

    vec3 F0 = vec3(0.2);
    vec3 F = fresnelSchlick(max(0.0, dot(V, N)), F0);
    vec2 reflectionTexCoord = vec2(texCoord.s, 1.0 - texCoord.t);
    vec4 reflectionColor = vec4(texture(reflectionMap, reflectionTexCoord + distortion).rgb, 1.0);

    vec3 waterColor = vec3(0.25, 0.4, 0.6);
    vec4 refractionColor = vec4(waterColor, min(waterDepth / turbidityDistance, 1.0));
    float alpha = F.r;//min(F.r, waterDepth);
    outColor = mix(refractionColor, reflectionColor, vec4(F, alpha));
}

