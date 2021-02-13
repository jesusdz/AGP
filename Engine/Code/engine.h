#pragma once

#include "platform.h"
#include <glad/glad.h>
#include <glm/glm.hpp>

typedef glm::vec2  vec2;
typedef glm::vec3  vec3;
typedef glm::vec4  vec4;
typedef glm::ivec2 ivec2;
typedef glm::ivec3 ivec3;
typedef glm::ivec4 ivec4;

struct Image
{
    void* pixels;
    ivec2 size;
    i32   nchannels;
    i32   stride;
};

struct Texture
{
    GLuint handle;
};

struct App
{
    // Loop
    f32  deltaTime;
    bool isRunning;

    // Graphics
    char gpuName[64];
    char openGlVersion[64];

    ivec2 displaySize;

    GLuint embeddedGeometryVertexBuffer;
    GLuint embeddedGeometryIndexBuffer;

    GLuint program;
    GLuint vao;

    Texture tex;

    GLuint programUniformTexture;

    bool takeSnapshot;
};

void Init(App* app);

void Gui(App* app);

void Update(App* app);

void Render(App* app);
