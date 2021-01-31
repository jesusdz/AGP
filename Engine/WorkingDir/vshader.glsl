#version 330

layout(location=0) in vec3 aPosition;

out vec3 vColor;

void main()
{
    vColor = aPosition;
    gl_Position = vec4(aPosition, 1.0);
}
