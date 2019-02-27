#version 330 core

in Data
{
    vec3 positionViewspace;
    vec3 normalViewspace;
} FSIn;

out vec4 outColor;

void main(void)
{
    vec3 L = vec3(0.0, 0.0, 1.0);
    vec3 N = FSIn.normalViewspace;
    vec3 albedo = vec3(1.0);
    float kD = max(0.0, dot(L, N));
    outColor.rgb = albedo * kD;
    outColor.a = 1.0;

    // Gamma correction
    outColor.rgb = pow(outColor.rgb, vec3(1.0/2.4));
}

