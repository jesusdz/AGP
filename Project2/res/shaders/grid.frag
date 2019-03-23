#version 330 core

uniform float left;
uniform float right;
uniform float bottom;
uniform float top;
uniform float znear;
uniform mat4 worldMatrix;
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;

in vec2 texCoord;
out vec4 outColor;

// grid lines
float grid(vec3 worldPos, float gridStep)
{
    // Compute world-space grid lines
    vec2 grid = fwidth(worldPos.xz) / abs(mod(worldPos.xz, gridStep));
    float line = step(1.0, max(grid.x, grid.y)); // binarize
    return line;
}

// attenuated grid lines
float agrid(vec3 worldPos, float gridStep, vec3 eyePos)
{
    float line = grid(worldPos, gridStep);
    float farFactor = gridStep / distance(worldPos, eyePos);
    float depthFactor = abs(eyePos.y) / distance(worldPos, eyePos);
    line = line * min(farFactor, depthFactor);
    return line;
}

void main()
{
    outColor = vec4(1.0, 1.0, 1.0, 1.0);

    // Eye direction
    vec3 eyedirEyespace;
    eyedirEyespace.x = left + texCoord.x * (right - left);
    eyedirEyespace.y = bottom + texCoord.y * (top - bottom);
    eyedirEyespace.z = -znear;
    vec3 eyedirWorldspace = normalize(mat3(worldMatrix) * eyedirEyespace);

    // Eye position
    vec3 eyeposEyespace = vec3(0, 0, 0);
    vec3 eyeposWorldspace = vec3(worldMatrix * vec4(eyeposEyespace, 1.0));

    // Plane parameters
    vec3 planeNormalWorldspace = vec3(0, 1, 0);
    vec3 planePointWorldspace = vec3(0, 0, 0);

    // Ray-plane intersection
    float numerator = dot(planePointWorldspace - eyeposWorldspace, planeNormalWorldspace);
    float denominator = dot(eyedirWorldspace, planeNormalWorldspace);
    float t = numerator / denominator;

    if (t > 0.0) // Intersected in front of the eye
    {
        vec3 hitWorldspace = eyeposWorldspace + eyedirWorldspace * t;

        // Color
        float line1 = agrid(hitWorldspace, 1.0, eyeposWorldspace);
        float line10 = agrid(hitWorldspace, 10.0, eyeposWorldspace);
        float line100 = agrid(hitWorldspace, 100.0, eyeposWorldspace);
        outColor = vec4(vec3(1.0), max(max(line1, line10), line100));

        // Small bias to avoid z-fighting
        vec3 bias = (eyeposWorldspace - hitWorldspace) * 0.005;

        // fragment depth
        vec4 hitClip = projectionMatrix * viewMatrix * vec4(hitWorldspace + bias, 1.0);
        float ndcDepth = hitClip.z / hitClip.w;
        gl_FragDepth = ((gl_DepthRange.diff * ndcDepth) + gl_DepthRange.near + gl_DepthRange.far) / 2.0;
    }
    else
    {
        gl_FragDepth = 0.0;
        discard;
    }
}
