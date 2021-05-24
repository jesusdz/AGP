//
// engine.h: This file contains the types and functions relative to the engine.
//

#pragma once

#include "platform.h"
#include <glad/glad.h>

using namespace glm;

#define MAX_RENDER_GROUPS 16
#define MAX_GPU_FRAME_DELAY 5
//#define USE_INSTANCING
#define MAX_RENDER_GROUP_CHILDREN_COUNT 16
#define MAX_RENDER_PRIMITIVES 4096
#define MAX_FRAMEBUFFER_ATTACHMENTS 16

struct RenderGroup
{
    const char* name;
    u32 children[MAX_RENDER_GROUP_CHILDREN_COUNT];
    u32 childrenCount;
};

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
    String filepath;
};

struct Material
{
    String name;
    vec3   albedo;
    vec3   emissive;
    f32    smoothness;
    u32    albedoTextureIdx;
    u32    emissiveTextureIdx;
    u32    specularTextureIdx;
    u32    normalsTextureIdx;
    u32    bumpTextureIdx;
};

struct VertexShaderAttribute
{
    u8 location;
    u8 componentCount;
};

#define MAX_ATTRIBUTE_COUNT 8

struct VertexShaderLayout
{
    VertexShaderAttribute attributes[MAX_ATTRIBUTE_COUNT];
    u8                    attributeCount;
};

struct VertexBufferAttribute
{
    u8 location;
    u8 componentCount;
    u8 offset;
};

struct VertexBufferLayout
{
    VertexBufferAttribute attributes[MAX_ATTRIBUTE_COUNT];
    u8                    attributeCount;
    u8                    stride;
};

struct Vao
{
    GLuint handle;
    GLuint programHandle;
    u32    meshIdx;
    u32    submeshIdx;
};

struct Buffer
{
    GLuint handle;
    GLenum type;
    u32    size;
    u32    head;
    void*  data; // mapped data
};

struct BufferRange
{
    u32 bufferIdx;
    u32 offset;
    u32 size;
};

struct Submesh
{
    VertexBufferLayout vertexBufferLayout;
    u32                vertexOffset;
    u32                indexOffset;
    u32                vertexCount;
    u32                indexCount;
};

struct Mesh
{
    std::vector<Submesh> submeshes;
    std::vector<u32>     materialIndices;
    Buffer               vertexBuffer;
    Buffer               indexBuffer;
};

struct Program
{
    GLuint             handle;
    VertexShaderLayout vertexInputLayout;
    String             filepath;
    String             programName;
    u64                lastWriteTimestamp;
};

enum RenderTargetType
{
    RenderTargetType_Color,
    RenderTargetType_Floats,
    RenderTargetType_Depth,
    RenderTargetType_Count
};

struct RenderTarget
{
    String           name;
    vec2             size;
    GLuint           handle;
    RenderTargetType type;
};

enum LoadOp { LoadOp_DontCare, LoadOp_Clear, LoadOp_Load };
enum StoreOp { StoreOp_DontCare, StoreOp_Store };

struct Attachment
{
    GLenum       attachmentPoint;
    u32          renderTargetIdx;
    LoadOp       loadOp;
    StoreOp      storeOp;
};

struct RenderPass
{
    GLuint       framebufferHandle;
    u32          attachmentCount;
    Attachment   attachments[MAX_FRAMEBUFFER_ATTACHMENTS];
};

struct RenderPrimitive
{
    u32    entityIdx;
    GLuint vaoHandle;
    GLuint albedoTextureHandle;
    u32    indexCount;
    u32    indexOffset;

#if defined(USE_INSTANCING)
    u32    instanceCount;
    u32    instancingOffset;
#else
    // If not using instancing
    u32    localParamsBufferIdx;
    u32    localParamsOffset;
    u32    localParamsSize;
#endif
};

struct ForwardRenderData
{
    u32    programIdx;
    GLuint uniLoc_Albedo;

    // Local params
    u32 localParamsBlockSize;

    Buffer instancingBuffer;

    // Render primitives
    RenderPrimitive renderPrimitives[MAX_RENDER_PRIMITIVES];
    u32             renderPrimitiveCount;
};

struct DeferredRenderData
{
    u32    gbufferProgramIdx;
    GLuint uniLoc_Albedo;

    u32    shadingProgramIdx;

    // Local params
    u32 localParamsBlockSize;

    Buffer instancingBuffer;

    // Render primitives
    RenderPrimitive renderPrimitives[MAX_RENDER_PRIMITIVES];
    u32             renderPrimitiveCount;
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
    mat4  viewMatrix;
    mat4  projectionMatrix;
    mat4  viewProjectionMatrix;
};

enum EntityType
{
    EntityType_Model,
    EntityType_Mesh
};

struct Entity
{
    EntityType type;
    mat4       worldMatrix;
    u32        meshSubmeshIdx; // meshIdx (16bits) and submeshIdx (16bits)
};

enum LightType
{
    LightType_Directional,
    LightType_Point
};

struct Light
{
    LightType type;
    vec3      color;
    vec3      direction;
    vec3      position;
};

enum RenderPath
{
    RenderPath_ForwardShading,
    RenderPath_DeferredShading,
    RenderPath_Count
};

struct Device
{
    // Basic info
    char name[64];
    char glVersionString[64];
    int  glVersion;
    int  glslVersion;

    // Extensions
    struct Extensions
    {
        bool GL_ARB_timer_query : 1;
    } ext;

    // Resources
    std::vector<Texture>      textures;
    std::vector<Material>     materials;
    std::vector<Mesh>         meshes;
    std::vector<Program>      programs;
    std::vector<Buffer>       constantBuffers;

    Vao          vaos[1024];
    u32          vaoCount;

    RenderTarget renderTargets[16];
    u32          renderTargetCount;

    RenderPass   renderPasses[16];
    u32          renderPassCount;

    // Capabilities
    GLint uniformBufferMaxSize;
    GLint uniformBufferAlignment;

    // For transient constant buffer storage...
    u32 currentConstantBufferIdx;
};

struct Embedded
{
    // Embedded meshes
    u32 meshIdx;
    u32 blitSubmeshIdx;
    u32 floorSubmeshIdx;
    u32 sphereSubmeshIdx;

    // Embedded textures
    u32 whiteTexIdx;
    u32 blackTexIdx;
    u32 normalTexIdx;
    u32 magentaTexIdx;
    u32 diceTexIdx;

    // Embedded materials
    u32 defaultMaterialIdx;

    // Textured geometry program
    u32    texturedGeometryProgramIdx;
    GLuint texturedGeometryProgram_TextureLoc;
};

struct DebugDraw
{
    Buffer opaqueLineVertexBuffer;
    Vao    opaqueLineVao;
    u32    opaqueLineCount;
    u32    opaqueProgramIdx;

    u32    texQuadCount;
    u32    texQuadTextureHandles[32];
    ivec4  texQuadRects[32];
};

#define MAX_ENTITIES 1024
#define MAX_LIGHTS    128

struct Scene
{
    u32 patrickModelIdx;

    Entity entities[MAX_ENTITIES];
    u32 entityCount;

    Light  lights[MAX_LIGHTS];
    u32 lightCount;

    Camera mainCamera;
};


struct App
{
    // Loop
    f32  deltaTime;
    bool isRunning;
    u32  frame;
    u32  frameMod;

    // Input
    Input input;

    ivec2 displaySize;

    Device device;

    Embedded embedded;

    DebugDraw debugDraw;

    ForwardRenderData forwardRenderData;

    DeferredRenderData deferredRenderData;

    Scene scene;

    // Render targets
    u32 albedoRenderTargetIdx;
    u32 normalRenderTargetIdx;
    u32 positionRenderTargetIdx;
    u32 radianceRenderTargetIdx;
    u32 depthRenderTargetIdx;

    // Render passes
    u32 gbufferPassIdx;
    u32 deferredShadingPassIdx;
    u32 forwardShadingPassIdx;

    // Global params
    u32 globalParamsBufferIdx;
    u32 globalParamsBlockSize;
    u32 globalParamsOffset;
    u32 globalParamsSize;

    // Mode
    RenderPath renderPath;

    bool takeSnapshot;

    u32         renderGroupCount;
    RenderGroup renderGroups[MAX_RENDER_GROUPS];
    GLuint      timerQueries[MAX_RENDER_GROUPS*MAX_GPU_FRAME_DELAY*2];

    u32         frameRenderGroup;
};

void Init(App* app);

void BeginFrame(App* app);

void Gui(App* app);

void Resize(App* app);

void Update(App* app);

void Render(App* app);

void EndFrame(App* app);

