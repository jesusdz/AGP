//
// engine.h: This file contains the types and functions relative to the engine.
//

#pragma once

#include "platform.h"
#include <glad/glad.h>

using namespace glm;

#define MAX_RENDER_GROUPS 16
#define MAX_GPU_FRAME_DELAY 5

struct RenderGroup
{
    const char* name;
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

enum RenderTargetType
{
    RenderTargetType_Color,
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

struct Attachment
{
    GLenum       attachmentPoint;
    u32          renderTargetIdx;
};

struct RenderPass
{
    GLuint       framebufferHandle;
    u32          attachmentCount;
    Attachment   attachments[16];
};

struct RenderPrimitive
{
    GLuint vaoHandle;
    GLuint albedoTextureHandle;
    u32    indexCount;
    u32    indexOffset;

	// If using instancing
    //u32    instanceCount;
    //u32    instancingOffset;

	// If not using instancing
    u32    localParamsBufferIdx;
    u32    localParamsOffset;
    u32    localParamsSize;
};

struct ForwardRenderData
{
    u32    programIdx;
    GLuint uniLoc_Albedo;

    // Local params
    u32 localParamsBlockSize;

    Buffer instancingBuffer;

    // Render primitives
    std::vector<RenderPrimitive> renderPrimitives;
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
    mat4 worldMatrix; // converts from local coords to world coords
    u32       modelIndex;
    u32       meshIndex;
    u32       submeshIndex;
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
    Mode_ForwardRender,
    Mode_Count
};

struct Device
{
    // Basic info
    char name[64];
    char glVersionString[64];
    int  glVersion;
    int  glslVersion;

    // Resources
    std::vector<Texture>      textures;
    std::vector<Material>     materials;
    std::vector<Mesh>         meshes;
    std::vector<Model>        models;
    std::vector<Program>      programs;
    std::vector<Buffer>       constantBuffers;
    std::vector<RenderTarget> renderTargets;
    std::vector<RenderPass>   renderPasses;

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

struct Scene
{
    u32 patrickModelIndex;

    std::vector<Entity> entities;
    std::vector<Light>  lights;
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

    Scene scene;

    // Render targets
    u32 colorRenderTargetIdx;
    u32 depthRenderTargetIdx;

    // Render passes
    u32 colorDepthPassIdx;

    // Global params
    u32 globalParamsBufferIdx;
    u32 globalParamsBlockSize;
    u32 globalParamsOffset;
    u32 globalParamsSize;

    // Mode
    Mode mode;
    u32  textureIndexShown;
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

