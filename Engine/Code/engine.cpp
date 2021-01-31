#pragma once

#include "engine.h"
#include <imgui.h>
#include <stb_image.h>

GLuint LoadProgram(String shaderSource, const char* shaderName)
{
    GLchar  infoLogBuffer[1024] = {};
    GLsizei infoLogBufferSize = sizeof(infoLogBuffer);
    GLsizei infoLogSize;
    GLint   success;

    char versionString[] = "#version 330\n";
    char shaderNameDefine[128];
    sprintf(shaderNameDefine, "#define %s\n", shaderName);
    char vertexShaderDefine[] = "#define VERTEX\n";
    char fragmentShaderDefine[] = "#define FRAGMENT\n";

    const GLchar* vertexShaderSource[] = {
        versionString,
        shaderNameDefine,
        vertexShaderDefine,
        shaderSource.str
    };
    const GLint vertexShaderLengths[] = {
        strlen(versionString),
        strlen(shaderNameDefine),
        strlen(vertexShaderDefine),
        shaderSource.length
    };
    const GLchar* fragmentShaderSource[] = {
        versionString,
        shaderNameDefine,
        fragmentShaderDefine,
        shaderSource.str
    };
    const GLint fragmentShaderLengths[] = {
        strlen(versionString),
        strlen(shaderNameDefine),
        strlen(fragmentShaderDefine),
        shaderSource.length
    };

    GLuint vshader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vshader, ARRAY_COUNT(vertexShaderSource), vertexShaderSource, vertexShaderLengths);
    glCompileShader(vshader);
    glGetShaderiv(vshader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vshader, infoLogBufferSize, &infoLogSize, infoLogBuffer);
        ELOG("glCompileShader() failed with vertex shader %s\nReported message:\n%s\n", shaderName, infoLogBuffer);
    }

    GLuint fshader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fshader, ARRAY_COUNT(fragmentShaderSource), fragmentShaderSource, fragmentShaderLengths);
    glCompileShader(fshader);
    glGetShaderiv(fshader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fshader, infoLogBufferSize, &infoLogSize, infoLogBuffer);
        ELOG("glCompileShader() failed with fragment shader %s\nReported message:\n%s\n", shaderName, infoLogBuffer);
    }

    GLuint program = glCreateProgram();
    glAttachShader(program, vshader);
    glAttachShader(program, fshader);
    glLinkProgram(program);
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(program, infoLogBufferSize, &infoLogSize, infoLogBuffer);
        ELOG("glLinkProgram() failed with program %s\nReported message:\n%s\n", shaderName, infoLogBuffer);
    }

    glUseProgram(0);

    glDetachShader(program, vshader);
    glDetachShader(program, fshader);
    glDeleteShader(vshader);
    glDeleteShader(fshader);

    return program;
}


Image LoadImage(const char* filename)
{
    Image img = {};
    img.pixels = stbi_loadf(filename, &img.size.x, &img.size.y, &img.nchannels, 4);
    img.stride = img.size.x * img.nchannels;
    return img;
}

Texture LoadTexture2D(const Image* image)
{
    GLenum internalFormat = GL_RGB8;
    GLenum dataFormat     = GL_RGB;
    GLenum dataType       = GL_UNSIGNED_BYTE;

    switch (image->nchannels)
    {
    case 3: dataFormat = GL_RGB; internalFormat = GL_RGB8; break;
    case 4: dataFormat = GL_RGBA; internalFormat = GL_RGBA8; break;
    default: ELOG("LoadTexture2D() - Unsupported number of channels");
    }

    Texture tex = {};
    glGenTextures(1, &tex.handle);
    glBindTexture(GL_TEXTURE_2D, tex.handle);
    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, image->size.x, image->size.y, 0, dataFormat, dataType, image->pixels);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glGenerateMipmap(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0);

    return tex;
}

void Init(App* app)
{
    sprintf(app->gpuName, "GPU: %s\n", glGetString(GL_RENDERER));
    sprintf(app->openGlVersion,"OpenGL & Driver version: %s\n", glGetString(GL_VERSION));

    struct VertexV3V2
    {
        glm::vec3 pos;
        glm::vec2 uv;
    };

    const VertexV3V2 vertices[] = {
        { glm::vec3(-0.5, -0.5, 0.0), glm::vec2(0.0, 0.0) }, // bottom-left vertex
        { glm::vec3( 0.5, -0.5, 0.0), glm::vec2(1.0, 0.0) }, // bottom-right vertex
        { glm::vec3( 0.5,  0.5, 0.0), glm::vec2(1.0, 1.0) }, // top-right vertex
        { glm::vec3(-0.5,  0.5, 0.0), glm::vec2(0.0, 1.0) }, // top-left vertex
    };

    const u16 indices[] = {
        0, 1, 2,
        0, 2, 3
    };

    // Geometry
    glGenBuffers(1, &app->embeddedGeometryBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, app->embeddedGeometryBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glGenBuffers(1, &app->embeddedGeometryIndexBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, app->embeddedGeometryIndexBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    // Attribute state
    glGenVertexArrays(1, &app->vao);
    glBindVertexArray(app->vao);
    glBindBuffer(GL_ARRAY_BUFFER, app->embeddedGeometryBuffer);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexV3V2), 0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(VertexV3V2), (void*)sizeof(vec3));
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, app->embeddedGeometryIndexBuffer);
    glBindVertexArray(0);

    
    // Pipeline
    String shaderSource = ReadTextFile("shaders.glsl");
    app->program = LoadProgram(shaderSource, "SIMPLE_SHADER");
    FreeString(shaderSource);
}

void Gui(App* app)
{
    ImGui::Begin("Info");
    ImGui::Text("GPU Name: %s", app->gpuName);
    ImGui::Text("OGL Version: %s", app->openGlVersion);
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

    //
    glViewport(0, 0, app->displaySize.x, app->displaySize.y);

    glUseProgram(app->program);
    glBindVertexArray(app->vao);

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);

    glBindVertexArray(0);
    glUseProgram(0);
}
