//
// engine.h: This file contains the types and functions relative to the engine.
//

#pragma once

#include "platform.h"
#include <glad/glad.h>

typedef glm::vec2  vec2;
typedef glm::vec3  vec3;
typedef glm::vec4  vec4;
typedef glm::ivec2 ivec2;
typedef glm::ivec3 ivec3;
typedef glm::ivec4 ivec4;

struct DebugGroup
{
    DebugGroup(const char* name) { glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 1, -1, name); }
    ~DebugGroup()                { glPopDebugGroup(); }
};

#define GL_DEBUG_GROUP(name) const DebugGroup debugGroup##__FILE__##__LINE__(name)

struct Image
{
    void* pixels;
    ivec2 size;
    i32   nchannels;
    i32   stride;
};

struct Texture
{
    GLuint      handle;
    std::string filepath;
};

struct Material
{
    std::string name;
    vec3        albedo;
    vec3        emissive;
    f32         smoothness;
    u32         albedoTextureIdx;
    u32         emissiveTextureIdx;
    u32         specularTextureIdx;
    u32         normalsTextureIdx;
    u32         bumpTextureIdx;
};

struct VertexShaderAttribute
{
    u8 location;
    u8 componentCount;
};

struct VertexShaderLayout
{
    std::vector<VertexShaderAttribute> attributes;
};

struct VertexBufferAttribute
{
    u8 location;
    u8 componentCount;
    u8 offset;
};

struct VertexBufferLayout
{
    std::vector<VertexBufferAttribute> attributes;
    u8                                 stride;
};

struct Vao
{
    GLuint handle;
    GLuint programHandle;
};

struct Submesh
{
    VertexBufferLayout vertexBufferLayout;
    std::vector<float> vertices;
    std::vector<u32>   indices;
    u32                vertexOffset;
    u32                indexOffset;

    std::vector<Vao>   vaos;
};

struct Mesh
{
    std::vector<Submesh> submeshes;
    GLuint               vertexBufferHandle;
    GLuint               indexBufferHandle;
};

struct Model
{
    u32              meshIdx;
    std::vector<u32> materialIdx;
};

struct Program
{
    GLuint             handle;
    VertexShaderLayout vertexInputLayout;
    std::string        filepath;
    std::string        programName;
    u64                lastWriteTimestamp;
};

struct RenderPass
{
    ivec2  framebufferSize;
    GLuint framebufferHandle;
    GLuint colorAttachmentHandle;
    GLuint depthAttachmentHandle;
};

struct Camera
{
    float yaw;
    float pitch;
    glm::vec3 position;
    glm::vec3 forward;
    glm::vec3 right;
    glm::vec3 speed;
};

enum Mode
{
    Mode_TexturedQuad,
    Mode_ModelNormals,
    Mode_ModelAlbedo,
    Mode_ModelShaded,
    Mode_Count
};

struct App
{
    // Loop
    f32  deltaTime;
    bool isRunning;

    // Input
    Input input;

    // Graphics
    char gpuName[64];
    char openGlVersion[64];

    ivec2 displaySize;

    u32  embeddedMeshIdx;

    u32    texturedGeometryProgramIdx;
    GLuint programUniformTexture;
    u32    diceTexIdx;

    u32     model;
    u32     meshProgramIdx;
    u32     texturedMeshProgramIdx;
    u32     transformedTexturedMeshProgramIdx;
    GLuint  texturedMeshProgram_uTexture;

    std::vector<Texture>    textures;
    std::vector<Material>   materials;
    std::vector<Mesh>       meshes;
    std::vector<Model>      models;
    std::vector<Program>    programs;
    std::vector<RenderPass> renderPasses;

    u32 whiteTexIdx;
    u32 blackTexIdx;
    u32 normalTexIdx;
    u32 magentaTexIdx;

    GLint uniformBufferMaxSize;
    GLint uniformBufferAlignment;
    GLuint uniformBuffer;

    Camera mainCamera;

    u32 forwardRenderPassIdx;

    // Mode
    Mode mode;
    u32  textureIndexShown;
    bool takeSnapshot;
};

void Init(App* app);

void Gui(App* app);

void Update(App* app);

void Render(App* app);

