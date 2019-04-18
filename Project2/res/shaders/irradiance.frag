#version 330 core

in vec3 localPosition;

uniform samplerCube environmentMap;
uniform int numTangentSamples;
uniform vec3 tangentSamples[1000];

const float PI = 3.14159265359;

out vec4 FragColor;

void main()
{
    // the sample direction equals the hemisphere's orientation
    vec3 normal = normalize(localPosition);
    vec3 up    = vec3(0.0, 1.0, 0.0);
    vec3 right = normalize(cross(up, normal));
    up         = cross(normal, right);

    vec3 irradiance = vec3(0.0);

#if 1
    float sampleDelta = 0.025;
    float nrSamples = 0.0;
    for(float phi = 0.0; phi < 2.0 * PI; phi += sampleDelta)
    {
        for(float theta = 0.0; theta < 0.5 * PI; theta += sampleDelta)
        {
            // spherical to cartesian (in tangent space)
            vec3 tangentSample = vec3(sin(theta) * cos(phi),  sin(theta) * sin(phi), cos(theta));
            // tangent space to world
            vec3 sampleVec = tangentSample.x * right + tangentSample.y * up + tangentSample.z * normal;

            irradiance += texture(environmentMap, sampleVec).rgb * cos(theta) * sin(theta);
            nrSamples++;
        }
    }
    irradiance = PI * (irradiance * (1.0 / float(nrSamples)));
#else
    for (int i = 0; i < numTangentSamples; ++i) {
        vec3 tangentSample = tangentSamples[i];
        vec3 worldSample = tangentSample.x * right + tangentSample.y * up + tangentSample.z * normal;
        irradiance += min(vec3(10.0), textureLod(environmentMap, worldSample, 4.0).rgb) * dot(worldSample, normal);
    }
    irradiance = PI * (irradiance * (1.0 / float(numTangentSamples)));
#endif

    //irradiance = textureLod(environmentMap, normal, 4.0).rgb;
    //irradiance = texture(environmentMap, normal).rgb;
    FragColor = vec4(irradiance, 1.0);
}
