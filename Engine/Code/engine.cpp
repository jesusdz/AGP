#pragma once

#include "engine.h"
#include <imgui.h>

GLuint loadProgram(const char* vertexShaderFilename, const char* fragmentShaderFilename)
{
    String vshaderStr = readTextFile(vertexShaderFilename);
    String fshaderStr = readTextFile(fragmentShaderFilename);

    GLchar  infoLogBuffer[1024] = {};
    GLsizei infoLogBufferSize = sizeof(infoLogBuffer);
    GLsizei infoLogSize;
    GLint   success;

    GLuint vshader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vshader, 1, &vshaderStr.str, (const GLint*)&vshaderStr.length);
    glCompileShader(vshader);
    glGetShaderiv(vshader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vshader, infoLogBufferSize, &infoLogSize, infoLogBuffer);
        ELOG("glCompileShader() failed with file %s\nReported message:\n%s\n", vertexShaderFilename, infoLogBuffer);
    }

    GLuint fshader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fshader, 1, &fshaderStr.str, (const GLint*)&fshaderStr.length);
    glCompileShader(fshader);
    glGetShaderiv(fshader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fshader, infoLogBufferSize, &infoLogSize, infoLogBuffer);
        ELOG("glCompileShader() failed with file %s\nReported message:\n%s\n", fragmentShaderFilename, infoLogBuffer);
    }

    GLuint program = glCreateProgram();
    glAttachShader(program, vshader);
    glAttachShader(program, fshader);
    glLinkProgram(program);
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(program, infoLogBufferSize, &infoLogSize, infoLogBuffer);
        ELOG("glLinkProgram() failed with files %s and %s\nReported message:\n%s\n", vertexShaderFilename, fragmentShaderFilename, infoLogBuffer);
    }

    glUseProgram(0);

    glDetachShader(program, vshader);
    glDetachShader(program, fshader);
    glDeleteShader(vshader);
    glDeleteShader(fshader);

    freeString(vshaderStr);
    freeString(fshaderStr);

    return program;
}

void Init(App* app)
{
    sprintf(app->gpuName, "GPU: %s\n", glGetString(GL_RENDERER));
    sprintf(app->openGlVersion,"OpenGL & Driver version: %s\n", glGetString(GL_VERSION));

    const glm::vec3 quadPositions[] = {
        glm::vec3(-0.5, -0.5, 0.0),
        glm::vec3( 0.5, -0.5, 0.0),
        glm::vec3( 0.5,  0.5, 0.0),
        glm::vec3(-0.5,  0.5, 0.0),
    };

    const u16 quadIndices[] = {
        0, 1, 2,
        0, 2, 3
    };

    // Geometry
    glGenBuffers(1, &app->embeddedGeometryBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, app->embeddedGeometryBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadPositions), quadPositions, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glGenBuffers(1, &app->embeddedGeometryIndexBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, app->embeddedGeometryIndexBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(quadIndices), quadIndices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    // Attribute state
    glGenVertexArrays(1, &app->vao);
    glBindVertexArray(app->vao);
    glBindBuffer(GL_ARRAY_BUFFER, app->embeddedGeometryBuffer);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, app->embeddedGeometryIndexBuffer);
    glBindVertexArray(0);

    // Pipeline
    app->program = loadProgram("vshader.glsl", "fshader.glsl");
}

void Gui(App* app)
{
    ImGui::Begin("Info");
    ImGui::Text("GPU Name: %s", app->gpuName);
    ImGui::Text("OGL Version: %s" , app->openGlVersion);
    ImGui::Text("FPS: %f", 1.0f/app->deltaTime);
    ImGui::End();
}

void Update(App* app)
{

}

void Render(App* app)
{
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glViewport(0, 0, app->displaySize.x, app->displaySize.y);

    glUseProgram(app->program);
    glBindVertexArray(app->vao);

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);

    glBindVertexArray(0);
    glUseProgram(0);
}
