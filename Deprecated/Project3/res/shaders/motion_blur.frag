#version 330 core

uniform sampler2D colorMap;
uniform sampler2D depthMap;
uniform mat4 viewProjectionMatrixInv;
uniform mat4 viewProjectionMatrixPrev;

in vec2 texCoords;

out vec4 outColor;

void main()
{
    // Get the depth buffer value at this pixel.
    float zOverW = texture(depthMap, texCoords).r;

    // H is the viewport position at this pixel in the range -1 to 1.
    vec4 H = vec4(texCoords.x * 2.0 - 1.0, texCoords.y * 2.0 - 1.0, zOverW * 2.0 - 1.0, 1.0);

    // Transform by the view-projection inverse.
    vec4 D = viewProjectionMatrixInv * H;

    // Divide by w to get the world position.
    vec3 worldPos = vec3(D / D.w);



    // Use the world position, and transform by the previous view-projection matrix.
    vec4 prevClipPos = viewProjectionMatrixPrev * vec4(worldPos, 1.0); // to clipspace
    vec3 prevNDCPos = vec3(prevClipPos / prevClipPos.w); // to ndc space
    vec2 prevTexCoords = vec2(prevNDCPos) * 0.5 + vec2(0.5);

    // Use this frame's position and last frame's to compute the pixel velocity.
    vec2 velocity = -vec2(texCoords - prevTexCoords);


    // Get the initial color at this pixel.
    vec4 color = texture(colorMap, texCoords);
    vec2 texCoord = texCoords + velocity;
    float sumSteps = 1.0;
    for(int i = 1; i < 10.0; ++i, texCoord += velocity/2)
    {
        if (texCoord.x < 0.0 || texCoord.x > 1.0 || texCoord.y < 0.0 || texCoord.y > 1.0) break;

        // Sample the color buffer along the velocity vector.
        vec4 currentColor = texture(colorMap, texCoord);
        texCoord += velocity;
        // Add the current color to our color sum.
        color += currentColor;
        sumSteps += 1.0;
    }
    // Average all of the samples to get the final blur color.
    vec4 finalColor = color / sumSteps;
    outColor = finalColor;
    outColor.a = 1.0;
    return;



    //vec4 color = texture(colorMap, texCoords);
    outColor = vec4(vec3(length(worldPos) / 10.0), 1.0);
    //outColor = vec4(velocity, vec2(1.0));
}
