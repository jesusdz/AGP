#version 330 core

in Data
{
    vec3 positionViewspace;
    vec3 normalViewspace;
} FSIn;

out vec4 outColor;

void main(void)
{
    vec3 L = -normalize(FSIn.positionViewspace);
    vec3 N = FSIn.normalViewspace;
    vec3 albedo = vec3(1.0);
    float kD = max(0.0, dot(L, N));
    outColor.rgb = albedo * kD;
    outColor.a = 1.0;

    // Fog
    vec3 fogColor = vec3(0.0, 0.0, 0.0);
    outColor.rgb = mix(outColor.rgb, fogColor, length(FSIn.positionViewspace)/50.0);

    // Gamma correction
    outColor.rgb = pow(outColor.rgb, vec3(1.0/2.4));
}

