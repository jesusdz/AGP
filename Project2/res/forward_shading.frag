#version 330 core

uniform vec4 albedo;
uniform vec4 emissive;
uniform float smoothness;

#define MAX_LIGHTS 8
uniform vec3 lightPosition[MAX_LIGHTS];
uniform vec3 lightColor[MAX_LIGHTS];
uniform int lightCount;

in Data
{
    vec3 positionViewspace;
    vec3 normalViewspace;
} FSIn;

out vec4 outColor;

void main(void)
{
    float fragDist = length(FSIn.positionViewspace);
    vec3 V = - FSIn.positionViewspace / fragDist;
    vec3 N = normalize(FSIn.normalViewspace);

    outColor.rgb = vec3(0.0);

    for (int i = 0; i < lightCount; ++i)
    {
        vec3 L = normalize(lightPosition[i] - FSIn.positionViewspace);
        float kD = max(0.0, dot(L, N));
#define BLINN_PHONG
#ifdef BLINN_PHONG
        vec3 H = normalize(L + V);
        float kS = pow(max(0.0, dot(H, N)), max(1.0, smoothness * 255.0));
#else
        vec3 R = reflect(-L, N);
        float kS = pow(max(0.0, dot(R, V)), max(1.0, smoothness * 255.0));
#endif
        outColor.rgb += albedo.rgb * kD + vec3(1.0) * kS;
    }

    //outColor.rgb += emissive.rgb;
    outColor.a = 1.0;

    // Fog
    vec3 fogColor = vec3(0.0, 0.0, 0.0);
    outColor.rgb = mix(outColor.rgb, fogColor, length(FSIn.positionViewspace)/50.0);

    // Gamma correction
    outColor.rgb = pow(outColor.rgb, vec3(1.0/2.4));
}

