#pragma once

#include "platform.h"
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <vector>

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

struct VertexShaderInput
{
    u8 location;
    u8 componentCount;
};

struct VertexShaderInputs
{
    std::vector<VertexShaderInput> attributes;
};

struct VertexBufferAttribute
{
    u8 location;
    u8 offset;
    u8 componentCount;
};

struct VertexBufferLayout
{
    std::vector<VertexBufferAttribute> attributes;
    u8                                 stride;
};

struct VaoInfo
{
    GLuint vao;
    GLuint program;
};

struct Submesh
{
    VertexBufferLayout vertexBufferLayout;
    std::vector<float> vertices;
    std::vector<u32>   indices;
    u32                vertexOffset;
    u32                indexOffset;

    std::vector<VaoInfo> vaoInfos;
};

struct Mesh
{
    std::vector<Submesh> submeshes;
    GLuint vertexBufferHandle;
    GLuint indexBufferHandle;
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
    GLuint programUniformTexture;
    GLuint vao;
    Texture tex;

    Mesh mesh;
    GLuint meshProgram;
    VertexShaderInputs meshProgramInput;

    bool takeSnapshot;
};

void Init(App* app);

void Gui(App* app);

void Update(App* app);

void Render(App* app);
