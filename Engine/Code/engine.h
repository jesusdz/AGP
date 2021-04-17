//
// engine.h: This file contains the types and functions relative to the engine.
//

#pragma once

#include "platform.h"
#include <glad/glad.h>

using namespace glm;

struct DebugGroup
{
    DebugGroup(const char* name) { if (glPushDebugGroup) glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 1, -1, name); }
    ~DebugGroup()                { if (glPopDebugGroup) glPopDebugGroup(); }
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

struct Buffer
{
    GLuint handle;
    GLenum type;
    u32    size;
    u32    head;
    void*  data; // mapped data
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
    Buffer               vertexBuffer;
    Buffer               indexBuffer;
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
    vec3  position;
    vec3  forward;
    vec3  right;
    vec3  speed;
    vec3  target;
};

enum EntityType
{
    EntityType_Model,
    EntityType_Mesh
};

struct Entity
{
    EntityType type;
    mat4 worldMatrix; // converts from local coords to world coords
    u32       modelIndex;
    u32       meshIndex;
    u32       submeshIndex;

    u32       localParamsBufferIdx;
    u32       localParamsOffset;
    u32       localParamsSize;
};

enum LightType
{
    LightType_Directional,
    LightType_Point
};

struct  Light
{
    LightType type;
    vec3      color;
    vec3      direction;
    vec3      position;
};

enum Mode
{
    Mode_BlitTexture,
    Mode_ModelNormals,
    Mode_ModelAlbedo,
    Mode_ForwardRender,
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
    char openGlVersionString[64];
    int  openGlMajorVersion;
    int  openGlMinorVersion;
    int  glslVersion;

    ivec2 displaySize;

    u32  embeddedMeshIdx;
    u32  blitSubmeshIdx;
    u32  floorSubmeshIdx;

    u32    texturedGeometryProgramIdx;
    GLuint programUniformTexture;
    u32    diceTexIdx;

    u32    debugDrawOpaqueProgramIdx;
    Buffer debugDrawOpaqueLineVertexBuffer;
    u32    debugDrawOpaqueLineCount;
    Vao    debugDrawOpaqueLineVao;

    u32     patrickModelIndex;
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
    std::vector<Buffer>     constantBuffers;

    u32 currentConstantBufferIdx;

    u32 whiteTexIdx;
    u32 blackTexIdx;
    u32 normalTexIdx;
    u32 magentaTexIdx;

    u32 defaultMaterialIdx;

    GLint uniformBufferMaxSize;
    GLint uniformBufferAlignment;

    u32 uniformBlockSize_GlobalParams;
    u32 uniformBlockSize_LocalParams;

    Camera mainCamera;

    u32 forwardRenderPassIdx;

    std::vector<Entity> entities;
    std::vector<Light>  lights;

    u32 globalParamsOffset;
    u32 globalParamsSize;

    // Mode
    Mode mode;
    u32  textureIndexShown;
    bool takeSnapshot;
};

void Init(App* app);

void Gui(App* app);

void Update(App* app);

void Render(App* app);

