//
// engine.cpp : Put all your graphics stuff in this file. This is kind of the graphics module.
// In here, you should type all your OpenGL commands, and you can also type code to handle
// input platform events (e.g to move the camera or react to certain shortcuts), writing some
// graphics related GUI options, and so on.
//

#include "engine.h"
#if USE_GFX_API_OPENGL
#include "ogl_engine.h"
#elif USE_GFX_API_METAL
#include "metal_engine.h"
#endif

#include <imgui.h>
#include <stb_image.h>
#include <stb_image_write.h>
#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#define BINDING(b) b

#define TIME_ELAPSED_QUERIES
//#define TIMESTAMP_QUERIES


static App* gApp = NULL;

static Arena StrArena = {};

static bool g_CullFace = true;


// https://www.khronos.org/opengl/wiki/Debug_Output
struct DebugEvent
{
    DebugEvent(App* app, u32 renderGroupIdx)
    {
#if USE_GFX_API_OPENGL
        if (glPushDebugGroup) glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 1, -1, app->renderGroups[renderGroupIdx].name);
#endif
    }
    ~DebugEvent()
    {
#if USE_GFX_API_OPENGL
        if (glPopDebugGroup) glPopDebugGroup();
#endif
    }
};

u32 GetTimerQueryIndex(u32 frameMod, u32 renderGroupIdx, u32 beginOrEnd)
{
    ASSERT(frameMod < MAX_GPU_FRAME_DELAY, "Frame mod out of bounds");
    ASSERT(renderGroupIdx < MAX_RENDER_GROUPS, "Render group out of bounds");
    ASSERT(beginOrEnd < 2, "An event can only have begin and end markers");
    u32 queryIdx = frameMod * MAX_RENDER_GROUPS * 2 + renderGroupIdx * 2 + beginOrEnd;
    return queryIdx;
}

void ProfileEvent_Init(App* app)
{
#if USE_GFX_API_OPENGL
    glGenQueries(ARRAY_COUNT(app->profileEventQueries), app->profileEventQueries);
#endif
}

void ProfileEvent_Insert(App* app, u32 renderGroup, ProfileEventType eventType)
{
#if USE_GFX_API_OPENGL
    const u32 renderGroupIdx = app->frameRenderGroup;

    ASSERT(app->profileEventCount < MAX_PROFILE_EVENTS, "Max number of timer queries reached");
    u32 profileEventIdx = (app->profileEventBegin + app->profileEventCount) % MAX_PROFILE_EVENTS;
    app->profileEventGroup[ profileEventIdx ] = renderGroup;
    app->profileEventTypes[ profileEventIdx ] = eventType;
#if defined(TIMESTAMP_QUERIES)
    const GLuint timerQuery = app->profileEventQueries[ profileEventIdx ];
    glQueryCounter(timerQuery, GL_TIMESTAMP);
#else
    if (eventType != ProfileEventType_FrameBegin)
    {
        glEndQuery(GL_TIME_ELAPSED);
    }
    if (eventType != ProfileEventType_FrameEnd)
    {
        const GLuint timeElapsedQuery = app->profileEventQueries[ profileEventIdx ];
        glBeginQuery(GL_TIME_ELAPSED, timeElapsedQuery);
    }
#endif
    app->profileEventCount++;
#endif
}

struct ProfileEvent
{
    App* _app;
    u32  _renderGroupIdx;

    ProfileEvent(App* app, u32 renderGroupIdx) : _app(app), _renderGroupIdx(renderGroupIdx)
    {
        ProfileEvent_Insert(app, renderGroupIdx, ProfileEventType_GroupBegin);
    }

    ~ProfileEvent()
    {
        ProfileEvent_Insert(_app, _renderGroupIdx, ProfileEventType_GroupEnd);
    }
};

u32 RegisterRenderGroup(App* app, const char* pName, u32 parentGroupIdx = 0xffffffff)
{
    ASSERT(app->renderGroupCount < MAX_RENDER_GROUPS, "MAX_RENDER_GROUPS limit reached");
    u32 groupIdx = app->renderGroupCount++;
    app->renderGroups[groupIdx].name = pName;

    // Parent child relationship
    if (parentGroupIdx != 0xffffffff)
    {
        RenderGroup& parentGroup = app->renderGroups[parentGroupIdx];
        ASSERT(parentGroup.childrenCount < ARRAY_COUNT(parentGroup.children), "Max render group children reached");
        parentGroup.children[parentGroup.childrenCount++] = groupIdx;
    }

    return groupIdx;
}

#define RENDER_GROUP(name, parentIdx) \
    static const u32   renderGroupIdx##__FILE__##__LINE__ = RegisterRenderGroup(gApp, name, parentIdx); \
    const DebugEvent   debugEvent    ##__FILE__##__LINE__(gApp, renderGroupIdx##__FILE__##__LINE__); \
    const ProfileEvent profileEvent  ##__FILE__##__LINE__(gApp, renderGroupIdx##__FILE__##__LINE__);

#include "buffers.cpp"

#if USE_GFX_API_OPENGL
GLuint CreateProgramFromSource(String programSource, int glslVersion, const char* shaderName)
{
    GLchar  infoLogBuffer[1024] = {};
    GLsizei infoLogBufferSize = sizeof(infoLogBuffer);
    GLsizei infoLogSize;
    GLint   success;

    ScratchArena arena;
    String glslVersionHeader    = FormatString(arena, "#version %u\n", glslVersion);
    String glslVersionDefine    = FormatString(arena, "#define VERSION %u\n", glslVersion);
    String shaderNameDefine     = FormatString(arena, "#define %s\n", shaderName);
    String vertexShaderDefine   = MakeString(arena, "#define VERTEX\n");
    String fragmentShaderDefine = MakeString(arena, "#define FRAGMENT\n");

    String defineUseInstancing  = MakeString(arena,
#if defined(USE_INSTANCING)
        "#define USE_INSTANCING\n"
#else
        ""
#endif
        );

    const GLchar* vertexShaderSource[] = {
        glslVersionHeader.str,
        glslVersionDefine.str,
        defineUseInstancing.str,
        shaderNameDefine.str,
        vertexShaderDefine.str,
        programSource.str
    };
    const GLint vertexShaderLengths[] = {
        (GLint) glslVersionHeader.len,
        (GLint) glslVersionDefine.len,
        (GLint) defineUseInstancing.len,
        (GLint) shaderNameDefine.len,
        (GLint) vertexShaderDefine.len,
        (GLint) programSource.len
    };
    const GLchar* fragmentShaderSource[] = {
        glslVersionHeader.str,
        glslVersionDefine.str,
        shaderNameDefine.str,
        fragmentShaderDefine.str,
        programSource.str
    };
    const GLint fragmentShaderLengths[] = {
        (GLint) glslVersionHeader.len,
        (GLint) glslVersionDefine.len,
        (GLint) shaderNameDefine.len,
        (GLint) fragmentShaderDefine.len,
        (GLint) programSource.len
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

    GLuint programHandle = glCreateProgram();
    glAttachShader(programHandle, vshader);
    glAttachShader(programHandle, fshader);
    glLinkProgram(programHandle);
    glGetProgramiv(programHandle, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(programHandle, infoLogBufferSize, &infoLogSize, infoLogBuffer);
        ELOG("glLinkProgram() failed with program %s\nReported message:\n%s\n", shaderName, infoLogBuffer);
    }

    glUseProgram(0);

    glDetachShader(programHandle, vshader);
    glDetachShader(programHandle, fshader);
    glDeleteShader(vshader);
    glDeleteShader(fshader);

#if 0
    // Shader reflection
    ILOG("SHADER REFLECTION FOR %s", shaderName);
    GLint   blockCount;
    GLchar  blockName[128];
    GLsizei blockNameLen;
    GLint   blockSize;
    GLint   blockUniformCount;
    GLint   blockUniformIndices[128];
    GLint   blockUniformOffsets[128];
    GLint   blockUniformCounts[128];
    GLchar  uniformName[128];
    GLsizei uniformNameLen;
    GLint   uniformCount;
    GLenum  uniformType;
    glGetProgramiv(programHandle, GL_ACTIVE_UNIFORM_BLOCKS, &blockCount);
    for (u32 blockIdx = 0; blockIdx < blockCount; ++blockIdx)
    {
        glGetActiveUniformBlockName(programHandle, blockIdx, ARRAY_COUNT(blockName), &blockNameLen, blockName);
        glGetActiveUniformBlockiv(programHandle, blockIdx, GL_UNIFORM_BLOCK_DATA_SIZE, &blockSize);
        ILOG("- Uniform Block: %s (%d Bytes)", blockName, blockSize);

        glGetActiveUniformBlockiv(programHandle, blockIdx, GL_UNIFORM_BLOCK_ACTIVE_UNIFORMS, &blockUniformCount );
        ASSERT(blockUniformCount <= ARRAY_COUNT(blockUniformIndices), "Uniform block exeeds the maximum number of fields");
        glGetActiveUniformBlockiv(programHandle, blockIdx, GL_UNIFORM_BLOCK_ACTIVE_UNIFORM_INDICES, blockUniformIndices );

        glGetActiveUniformsiv(programHandle, blockUniformCount, (const GLuint*)blockUniformIndices, GL_UNIFORM_OFFSET, blockUniformOffsets);
        glGetActiveUniformsiv(programHandle, blockUniformCount, (const GLuint*)blockUniformIndices, GL_UNIFORM_SIZE, blockUniformCounts);

        for (u32 blockUniformIdx = 0; blockUniformIdx < blockUniformCount; blockUniformIdx++)
        {
            u32 uniformIdx = blockUniformIndices[blockUniformIdx];

            glGetActiveUniform(programHandle, uniformIdx, ARRAY_COUNT(uniformName), &uniformNameLen, &uniformCount, &uniformType, uniformName);
            ILOG("  -> uniform: %s (offset:%d count:%d)", uniformName, blockUniformOffsets[blockUniformIdx], blockUniformCounts[blockUniformIdx]);
        }
    }
#endif

    return programHandle;
}
#endif

#if USE_GFX_API_OPENGL
VertexShaderLayout ExtractVertexShaderLayoutFromProgram(GLuint programHandle)
{
    VertexShaderLayout layout = {};

    GLint attributeCount = 0;
    char attributeName[128];
    GLsizei attributeNameLength;
    GLint attributeSize;
    GLenum attributeType;
    GLint attributeLocation;
    u8 attributeComponentCount = 0;

    glGetProgramiv(programHandle, GL_ACTIVE_ATTRIBUTES, &attributeCount);

    for (i32 i = 0; i < attributeCount; ++i)
    {
        glGetActiveAttrib(programHandle, i,
                          ARRAY_COUNT(attributeName),
                          &attributeNameLength,
                          &attributeSize,
                          &attributeType,
                          attributeName);

        attributeLocation = glGetAttribLocation(programHandle, attributeName);

        const GLint VertexStream_FirstInstancingStream = 6;
        if (attributeLocation >= VertexStream_FirstInstancingStream)
            continue;

        switch (attributeType)
        {
            case GL_FLOAT:      attributeComponentCount = 1; break;
            case GL_FLOAT_VEC2: attributeComponentCount = 2; break;
            case GL_FLOAT_VEC3: attributeComponentCount = 3; break;
            case GL_FLOAT_VEC4: attributeComponentCount = 4; break;
            default: INVALID_CODE_PATH("Unsupported attribute type");
        }

        ASSERT(layout.attributeCount < ARRAY_COUNT(layout.attributes), "Max number of attributes reached.");
        layout.attributes[layout.attributeCount++] = {
            (u8)attributeLocation,
            (u8)attributeComponentCount
            };
    }

    return layout;
}
#endif


u32 LoadProgram(Device& device, String filepath, String programName)
{
    String programSource = ReadTextFile(filepath.str);

    Program program = {};
#if USE_GFX_API_OPENGL
    program.handle = CreateProgramFromSource(programSource, device.glslVersion, programName.str);
    program.vertexInputLayout = ExtractVertexShaderLayoutFromProgram(program.handle);
#endif
    program.filepath = filepath;
    program.programName = programName;
    program.lastWriteTimestamp = GetFileLastWriteTimestamp(filepath.str);
    device.programs[device.programCount++] = program;

    return device.programCount - 1;
}

Image LoadImage(const char* filename)
{
    Image img = {};
    stbi_set_flip_vertically_on_load(true);
    img.pixels = stbi_load(filename, &img.size.x, &img.size.y, &img.nchannels, 0);
    if (img.pixels)
    {
        img.stride = img.size.x * img.nchannels;
    }
    else
    {
        ELOG("Could not open file %s", filename);
    }
    return img;
}

void FreeImage(Image image)
{
    stbi_image_free(image.pixels);
}

#if USE_GFX_API_OPENGL
GLuint CreateTexture2DFromImage(Image image)
{
    GLenum internalFormat = GL_RGB8;
    GLenum dataFormat     = GL_RGB;
    GLenum dataType       = GL_UNSIGNED_BYTE;

    switch (image.nchannels)
    {
        case 3: dataFormat = GL_RGB; internalFormat = GL_RGB8; break;
        case 4: dataFormat = GL_RGBA; internalFormat = GL_RGBA8; break;
        default: ELOG("LoadTexture2D() - Unsupported number of channels");
    }

    GLuint texHandle;
    glGenTextures(1, &texHandle);
    glBindTexture(GL_TEXTURE_2D, texHandle);
    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, image.size.x, image.size.y, 0, dataFormat, dataType, image.pixels);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glGenerateMipmap(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0);

    return texHandle;
}
#endif

u32 LoadTexture2D(Device& device, const char* filepath)
{
    for (u32 texIdx = 0; texIdx < device.textureCount; ++texIdx)
        if (SameString(device.textures[texIdx].filepath, CString(filepath)))
            return texIdx;

    Image image = LoadImage(filepath);

    if (image.pixels)
    {
        Texture tex = {};
#if USE_GFX_API_OPENGL
        tex.handle = CreateTexture2DFromImage(image);
#endif
        tex.filepath = InternString(StrArena, filepath);

        ASSERT(device.textureCount < ARRAY_COUNT(device.textures), "Max number of textures reached");
        u32 texIdx = device.textureCount;
        device.textures[device.textureCount++] = tex;

        FreeImage(image);
        return texIdx;
    }
    else
    {
        return UINT32_MAX;
    }
}

void ProcessAssimpMesh(const aiScene* scene, aiMesh *mesh, Mesh *myMesh, u32 baseMeshMaterialIdx, std::vector<u32>& submeshMaterialIndices, Arena& vertexArena, Arena& indexArena)
{
    bool hasTexCoords = false;
    bool hasTangentSpace = false;

    const u32 vertexOffset = vertexArena.head;
    const u32 indexOffset = indexArena.head;

    // process vertices
    for(unsigned int i = 0; i < mesh->mNumVertices; i++)
    {
        PushFloat(vertexArena, mesh->mVertices[i].x);
        PushFloat(vertexArena, mesh->mVertices[i].y);
        PushFloat(vertexArena, mesh->mVertices[i].z);
        PushFloat(vertexArena, mesh->mNormals[i].x);
        PushFloat(vertexArena, mesh->mNormals[i].y);
        PushFloat(vertexArena, mesh->mNormals[i].z);

        if(mesh->mTextureCoords[0]) // does the mesh contain texture coordinates?
        {
            hasTexCoords = true;
            PushFloat(vertexArena, mesh->mTextureCoords[0][i].x);
            PushFloat(vertexArena, mesh->mTextureCoords[0][i].y);
        }

        if(mesh->mTangents != nullptr && mesh->mBitangents)
        {
            hasTangentSpace = true;
            PushFloat(vertexArena, mesh->mTangents[i].x);
            PushFloat(vertexArena, mesh->mTangents[i].y);
            PushFloat(vertexArena, mesh->mTangents[i].z);

            // For some reason ASSIMP gives me the bitangents flipped.
            // Maybe it's my fault, but when I generate my own geometry
            // in other files (see the generation of standard assets)
            // and all the bitangents have the orientation I expect,
            // everything works ok.
            // I think that (even if the documentation says the opposite)
            // it returns a left-handed tangent space matrix.
            // SOLUTION: I invert the components of the bitangent here.
            PushFloat(vertexArena, -mesh->mBitangents[i].x);
            PushFloat(vertexArena, -mesh->mBitangents[i].y);
            PushFloat(vertexArena, -mesh->mBitangents[i].z);
        }
    }

    // process indices
    for(unsigned int i = 0; i < mesh->mNumFaces; i++)
    {
        aiFace face = mesh->mFaces[i];
        ASSERT(face.mNumIndices == 3, "Faces should have three vertices");
        for(unsigned int j = 0; j < face.mNumIndices; j++)
        {
            PushU32(indexArena, face.mIndices[j]);
        }
    }

    // store the proper (previously proceessed) material for this mesh
    submeshMaterialIndices.push_back(baseMeshMaterialIdx + mesh->mMaterialIndex);

    // create the vertex format
    u8 attCount = 0;
    VertexBufferLayout vertexBufferLayout = {};
    vertexBufferLayout.attributes[attCount++] = VertexBufferAttribute{ 0, 3, 0 };
    vertexBufferLayout.attributes[attCount++] = VertexBufferAttribute{ 1, 3, 3*sizeof(float) };
    vertexBufferLayout.stride = 6 * sizeof(float);
    if (hasTexCoords)
    {
        vertexBufferLayout.attributes[attCount++] = VertexBufferAttribute{ 2, 2, vertexBufferLayout.stride };
        vertexBufferLayout.stride += 2 * sizeof(float);
    }
    if (hasTangentSpace)
    {
        vertexBufferLayout.attributes[attCount++] = VertexBufferAttribute{ 3, 3, vertexBufferLayout.stride };
        vertexBufferLayout.stride += 3 * sizeof(float);

        vertexBufferLayout.attributes[attCount++] = VertexBufferAttribute{ 4, 3, vertexBufferLayout.stride };
        vertexBufferLayout.stride += 3 * sizeof(float);
    }
    vertexBufferLayout.attributeCount = attCount;

   // add the submesh into the mesh
   Submesh submesh = {};
   submesh.vertexBufferLayout = vertexBufferLayout;
   submesh.vertexOffset = vertexOffset;
   submesh.indexOffset = indexOffset;
   submesh.vertexCount = mesh->mNumVertices;
   submesh.indexCount = mesh->mNumFaces*3;
   myMesh->submeshes.push_back( submesh );
}

void ProcessAssimpMaterial(Device& device, aiMaterial *material, Material& myMaterial, String directory)
{
    aiString name;
    aiColor3D diffuseColor;
    aiColor3D emissiveColor;
    aiColor3D specularColor;
    ai_real shininess;
    material->Get(AI_MATKEY_NAME, name);
    material->Get(AI_MATKEY_COLOR_DIFFUSE, diffuseColor);
    material->Get(AI_MATKEY_COLOR_EMISSIVE, emissiveColor);
    material->Get(AI_MATKEY_COLOR_SPECULAR, specularColor);
    material->Get(AI_MATKEY_SHININESS, shininess);

    myMaterial.name = InternString(StrArena, name.C_Str());
    myMaterial.albedo = vec3(diffuseColor.r, diffuseColor.g, diffuseColor.b);
    myMaterial.emissive = vec3(emissiveColor.r, emissiveColor.g, emissiveColor.b);
    myMaterial.smoothness = shininess / 256.0f;

    ScratchArena TmpArena;

    aiString aiFilename;
    if (material->GetTextureCount(aiTextureType_DIFFUSE) > 0)
    {
        material->GetTexture(aiTextureType_DIFFUSE, 0, &aiFilename);
        String filename = CString(aiFilename.C_Str());
        String filepath = MakePath(TmpArena, directory, filename);
        myMaterial.albedoTextureIdx = LoadTexture2D(device, filepath.str);
    }
    if (material->GetTextureCount(aiTextureType_EMISSIVE) > 0)
    {
        material->GetTexture(aiTextureType_EMISSIVE, 0, &aiFilename);
        String filename = CString(aiFilename.C_Str());
        String filepath = MakePath(TmpArena, directory, filename);
        myMaterial.emissiveTextureIdx = LoadTexture2D(device, filepath.str);
    }
    if (material->GetTextureCount(aiTextureType_SPECULAR) > 0)
    {
        material->GetTexture(aiTextureType_SPECULAR, 0, &aiFilename);
        String filename = CString(aiFilename.C_Str());
        String filepath = MakePath(TmpArena, directory, filename);
        myMaterial.specularTextureIdx = LoadTexture2D(device, filepath.str);
    }
    if (material->GetTextureCount(aiTextureType_NORMALS) > 0)
    {
        material->GetTexture(aiTextureType_NORMALS, 0, &aiFilename);
        String filename = CString(aiFilename.C_Str());
        String filepath = MakePath(TmpArena, directory, filename);
        myMaterial.normalsTextureIdx = LoadTexture2D(device, filepath.str);
    }
    if (material->GetTextureCount(aiTextureType_HEIGHT) > 0)
    {
        material->GetTexture(aiTextureType_HEIGHT, 0, &aiFilename);
        String filename = CString(aiFilename.C_Str());
        String filepath = MakePath(TmpArena, directory, filename);
        myMaterial.bumpTextureIdx = LoadTexture2D(device, filepath.str);
    }

    //myMaterial.createNormalFromBump();
}

void ProcessAssimpNode(const aiScene* scene, aiNode *node, Mesh *myMesh, u32 baseMeshMaterialIdx, std::vector<u32>& submeshMaterialIndices, Arena& vertexArena, Arena& indexArena)
{
    // process all the node's meshes (if any)
    for(unsigned int i = 0; i < node->mNumMeshes; i++)
    {
        aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
        ProcessAssimpMesh(scene, mesh, myMesh, baseMeshMaterialIdx, submeshMaterialIndices, vertexArena, indexArena);
    }

    // then do the same for each of its children
    for(unsigned int i = 0; i < node->mNumChildren; i++)
    {
        ProcessAssimpNode(scene, node->mChildren[i], myMesh, baseMeshMaterialIdx, submeshMaterialIndices, vertexArena, indexArena);
    }
}

u32 LoadModel(Device& device, const char* filename)
{
    const aiScene* scene = aiImportFile(filename,
                                        aiProcess_Triangulate           |
                                        aiProcess_GenSmoothNormals      |
                                        aiProcess_CalcTangentSpace      |
                                        aiProcess_JoinIdenticalVertices |
                                        aiProcess_PreTransformVertices  |
                                        aiProcess_ImproveCacheLocality  |
                                        aiProcess_OptimizeMeshes        |
                                        aiProcess_SortByPType);

    if (!scene)
    {
        ELOG("Error loading mesh %s: %s", filename, aiGetErrorString());
        return UINT32_MAX;
    }

    ASSERT(device.meshCount < ARRAY_COUNT(device.meshes), "Max number of meshes reached");
    u32 meshIdx = device.meshCount++;
    Mesh& mesh = device.meshes[meshIdx];
    mesh = Mesh{};

    ScratchArena TmpArena;

    String directory = GetDirectoryPart(MakeString(TmpArena, filename));

    // Create a list of materials
    const u32 baseMeshMaterialIdx = (u32)device.materialCount;
    for (unsigned int i = 0; i < scene->mNumMaterials; ++i)
    {
        ASSERT(device.materialCount < ARRAY_COUNT(device.materials), "Max number of materials reached");
        Material& material = device.materials[device.materialCount];
        device.materials[device.materialCount++] = Material{};
        ProcessAssimpMaterial(device, scene->mMaterials[i], material, directory);
    }

    //IndexAndVertexCount result = ProcessAssimpNode_IndexAndVertexCount(scene, scene->mRootNode);

    ScratchArena vertexArena;
    ScratchArena indexArena;
    ProcessAssimpNode(scene, scene->mRootNode, &mesh, baseMeshMaterialIdx, mesh.materialIndices, vertexArena, indexArena);

    aiReleaseImport(scene);

    const u32 vertexBufferSize = vertexArena.head;
    const u32 indexBufferSize = indexArena.head;

    mesh.vertexBufferIdx = CreateStaticVertexBuffer(device, vertexBufferSize);
    mesh.indexBufferIdx = CreateStaticIndexBuffer(device, indexBufferSize);

    Buffer& vertexBuffer = device.vertexBuffers[mesh.vertexBufferIdx];
    Buffer& indexBuffer = device.indexBuffers[mesh.indexBufferIdx];

    MapBuffer(vertexBuffer, Access_Write);
    MapBuffer(indexBuffer, Access_Write);

    BufferPushData(vertexBuffer, vertexArena.data, vertexBufferSize);
    BufferPushData(indexBuffer, indexArena.data, indexBufferSize);

    UnmapBuffer(vertexBuffer);
    UnmapBuffer(indexBuffer);

    return meshIdx;
}

Vao CreateVAORaw(const Buffer&             indexBuffer,
                 const Buffer&             vertexBuffer,
                 u32                       vertexBufferOffset,
                 // TODO:
                 // const Buffer& instanceBuffer
                 // u32           instanceBufferOffset
                 const VertexBufferLayout& bufferLayout,
                 const VertexShaderLayout& shaderLayout,
                 const Program&            shaderProgram,
                 u32                       meshIdx,
                 u32                       submeshIdx)
{
#if USE_GFX_API_OPENGL
    // Create a new vao for this submesh/program
    GLuint vaoHandle = 0;
    glGenVertexArrays(1, &vaoHandle);
    glBindVertexArray(vaoHandle);

    BindBuffer(vertexBuffer);
    BindBuffer(indexBuffer);

    // We have to link all vertex inputs attributes to attributes in the vertex buffer
    for (u32 i = 0; i < shaderLayout.attributeCount; ++i)
    {
        bool attributeWasLinked = false;

        // TODO: If it is a per-vertex attribute

        for (u32 j = 0; j < bufferLayout.attributeCount; ++j)
        {
            if (shaderLayout.attributes[i].location == bufferLayout.attributes[j].location)
            {
                const u32 index  = bufferLayout.attributes[j].location;
                const u32 ncomp  = bufferLayout.attributes[j].componentCount;
                const u32 offset = bufferLayout.attributes[j].offset + vertexBufferOffset; // attribute offset + vertex offset
                const u32 stride = bufferLayout.stride;
                glVertexAttribPointer(index, ncomp, GL_FLOAT, GL_FALSE, stride, (void*)(u64)offset);
                glEnableVertexAttribArray(index);

                attributeWasLinked = true;
                break;
            }
        }

        ASSERT(attributeWasLinked, "The submesh should provide an attribute for each vertex input");

        // TODO: Else, check the instance buffer
    }

    glBindVertexArray(0);

    // Store it in the list of vaos for this submesh
    Vao vao = { vaoHandle, shaderProgram.handle, meshIdx, submeshIdx };
    return vao;
#else
    Vao vao = { };
    return vao;
#endif
}

Vao CreateVAORaw(const Buffer& vertexBuffer,
                 u32           vertexBufferOffset,
                 const VertexBufferLayout& bufferLayout,
                 const VertexShaderLayout& shaderLayout,
                 const Program& shaderProgram,
                 u32            meshIdx,
                 u32            submeshIdx)
{
    Buffer invalidIndexBuffer = {};
    return CreateVAORaw(invalidIndexBuffer, vertexBuffer, vertexBufferOffset, bufferLayout, shaderLayout, shaderProgram, meshIdx, submeshIdx);
}

#if USE_GFX_API_OPENGL
GLuint FindVAO(Device& device, u32 meshIdx, u32 submeshIdx, const Program& program)
{
    Mesh& mesh = device.meshes[meshIdx];
    Submesh& submesh = mesh.submeshes[submeshIdx];

    // Try finding a vao for this submesh/program
    for (u32 i = 0; i < device.vaoCount; ++i)
        if (device.vaos[i].programHandle == program.handle && meshIdx == device.vaos[i].meshIdx && submeshIdx == device.vaos[i].submeshIdx)
            return device.vaos[i].handle;

    ASSERT(device.vaoCount < ARRAY_COUNT(device.vaos), "Max number of vaos reached");
    Buffer& vertexBuffer = device.vertexBuffers[mesh.vertexBufferIdx];
    Buffer& indexBuffer = device.indexBuffers[mesh.indexBufferIdx];
    Vao vao = CreateVAORaw(indexBuffer, vertexBuffer, submesh.vertexOffset, submesh.vertexBufferLayout, program.vertexInputLayout, program, meshIdx, submeshIdx);
    device.vaos[device.vaoCount++] = vao;

    return vao.handle;
}
#endif

RenderTarget CreateRenderTargetRaw(String name, ivec2 displaySize, RenderTargetType type)
{
#if USE_GFX_API_OPENGL
    GLenum internalFormat = GL_RGBA8;
    GLenum externalFormat = GL_RGBA;
    GLenum channelDataType = GL_UNSIGNED_BYTE;

    if (type == RenderTargetType_Floats)
    {
        internalFormat = GL_RGBA32F;
        externalFormat = GL_RGBA;
        channelDataType = GL_FLOAT;
    }
    else if (type == RenderTargetType_Depth)
    {
        internalFormat = GL_DEPTH_COMPONENT24;
        externalFormat = GL_DEPTH_COMPONENT;
        channelDataType = GL_FLOAT;
    }

    // Framebuffer
    GLuint textureHandle;
    glGenTextures(1, &textureHandle);
    glBindTexture(GL_TEXTURE_2D, textureHandle);
    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, displaySize.x, displaySize.y, 0, externalFormat, channelDataType, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_2D, 0);
#endif

    RenderTarget renderTarget = {};
    renderTarget.name         = name;
    renderTarget.size         = displaySize;
#if USE_GFX_API_OPENGL
    renderTarget.handle       = textureHandle;
#endif
    renderTarget.type         = type;
    return renderTarget;
}

u32 CreateRenderTarget(Device& device, String name, RenderTargetType type, ivec2 size)
{
    ASSERT(device.renderTargetCount < ARRAY_COUNT(device.renderTargets), "Max number of render targets reached.");
    RenderTarget renderTarget = CreateRenderTargetRaw(name, size, type);
    device.renderTargets[device.renderTargetCount++] = renderTarget;
    return device.renderTargetCount - 1;
}

void DestroyRenderTargetRaw(const RenderTarget& renderTarget)
{
#if USE_GFX_API_OPENGL
    glDeleteTextures(1, &renderTarget.handle);
#endif
}

Framebuffer CreateFramebufferRaw(Device& device, u32 attachmentCount, Attachment* attachments)
{
#if USE_GFX_API_OPENGL
    GLuint framebufferHandle;
    glGenFramebuffers(1, &framebufferHandle);
    glBindFramebuffer(GL_FRAMEBUFFER, framebufferHandle);

    for (u32 i = 0; i < attachmentCount; ++i)
    {
        const GLenum attachmentEnum  = GLenumFromAttachmentPoint[attachments[i].attachmentPoint];
        const u32    renderTargetIdx = attachments[i].renderTargetIdx;
        const RenderTarget& renderTarget = device.renderTargets[ renderTargetIdx ];
        glFramebufferTexture(GL_FRAMEBUFFER, attachmentEnum, renderTarget.handle, 0);
    }

    GLenum framebufferStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (framebufferStatus != GL_FRAMEBUFFER_COMPLETE)
    {
        switch (framebufferStatus)
        {
            case GL_FRAMEBUFFER_UNDEFINED:                     ELOG("GL_FRAMEBUFFER_UNDEFINED"); break;
            case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:         ELOG("GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT"); break;
            case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT: ELOG("GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT"); break;
            case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:        ELOG("GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER"); break;
            case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:        ELOG("GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER"); break;
            case GL_FRAMEBUFFER_UNSUPPORTED:                   ELOG("GL_FRAMEBUFFER_UNSUPPORTED"); break;
            case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:        ELOG("GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE"); break;
            case GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS:      ELOG("GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS"); break;
            default: ELOG("Unknown framebuffer status error");
        }
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
#endif

    Framebuffer framebuffer = {};
#if USE_GFX_API_OPENGL
    framebuffer.handle = framebufferHandle;
#endif
    framebuffer.attachmentCount = attachmentCount;
    MemCopy(&framebuffer.attachments, attachments, attachmentCount*sizeof(Attachment));
    return framebuffer;
}

void DestroyFramebufferRaw(const Framebuffer& framebuffer)
{
#if USE_GFX_API_OPENGL
    glDeleteFramebuffers(1, &framebuffer.handle);
#endif
}

u32 CreateFramebuffer(Device& device, u32 attachmentCount, Attachment* attachments)
{
    ASSERT(device.framebufferCount < ARRAY_COUNT(device.framebuffers), "Max number of render passes reached");
    Framebuffer framebuffer = CreateFramebufferRaw(device, attachmentCount, attachments);
    device.framebuffers[device.framebufferCount++] = framebuffer;
    return device.framebufferCount - 1;
}

RenderPass CreateRenderPassRaw(Device& device, u32 framebufferIdx, u32 attachmentActionCount, AttachmentAction* attachmentActions)
{
    RenderPass renderPass = {};
    renderPass.framebufferIdx = framebufferIdx;
    renderPass.attachmentActionCount = attachmentActionCount;
    MemCopy(&renderPass.attachmentActions, attachmentActions, attachmentActionCount*sizeof(AttachmentAction));
    return renderPass;
}

u32 CreateRenderPass(Device& device, u32 framebufferIdx, u32 attachmentActionCount, AttachmentAction* attachmentActions)
{
    ASSERT(device.renderPassCount < ARRAY_COUNT(device.renderPasses), "Max number of render passes reached");
    RenderPass renderPass = CreateRenderPassRaw(device, framebufferIdx, attachmentActionCount, attachmentActions);
    device.renderPasses[device.renderPassCount++] = renderPass;
    return device.renderPassCount - 1;
}

void BeginRenderPass( const Device& device, u32 renderPassIdx )
{
#if USE_GFX_API_OPENGL
    const RenderPass& renderPass = device.renderPasses[renderPassIdx];
    const Framebuffer& framebuffer = device.framebuffers[renderPass.framebufferIdx];
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer.handle);

    GLuint colorBuffers[MAX_FRAMEBUFFER_ATTACHMENTS] = {};
    u32 colorBufferCount = 0;

    for (u32 i = 0; i < renderPass.attachmentActionCount; ++i)
    {
        const AttachmentAction& action = renderPass.attachmentActions[i];
        const Attachment& attachment = framebuffer.attachments[action.attachmentIdx];

        if (action.loadOp == LoadOp_Clear)
        {
            if (attachment.attachmentPoint < Attachment_Depth)
            {
                const GLenum attachmentEnum = GLenumFromAttachmentPoint[attachment.attachmentPoint];
                glDrawBuffer(attachmentEnum);
                glClearColor(0.1f, 0.1f, 0.1f, 1.0f); // TODO: Avoid hardcoding this clear color
                glClear(GL_COLOR_BUFFER_BIT);
            }
            else
            {
                glClear(GL_DEPTH_BUFFER_BIT);
            }
        }

        if (attachment.attachmentPoint < Attachment_Depth) {
            const GLenum attachmentEnum = GLenumFromAttachmentPoint[attachment.attachmentPoint];
            colorBuffers[colorBufferCount++] = attachmentEnum;
        }
    }

    if (colorBufferCount > 0)
        glDrawBuffers(colorBufferCount, colorBuffers);
#endif
}

void EndRenderPass( const Device& )
{
#if USE_GFX_API_OPENGL
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
#endif
}

void AddEntity(Scene& scene, const Entity& entity)
{
    ASSERT(scene.entityCount < ARRAY_COUNT(scene.entities), "Reached max number of entities");
    scene.entities[scene.entityCount++] = entity;
}

void AddModelEntity(Scene& scene, u32 meshIdx, const mat4& worldMatrix)
{
    Entity entity = {};
    entity.type = EntityType_Model;
    entity.meshSubmeshIdx = MAKE_DWORD(meshIdx, 0);
    entity.worldMatrix = worldMatrix;
    AddEntity(scene, entity);
}

void AddMeshEntity(Scene& scene, u32 meshIdx, u32 submeshIdx, const mat4& worldMatrix)
{
    Entity entity = {};
    entity.type = EntityType_Mesh;
    entity.meshSubmeshIdx = MAKE_DWORD(meshIdx, submeshIdx);
    entity.worldMatrix = worldMatrix;
    AddEntity(scene, entity);
}

void AddLight(Scene& scene, const Light& light)
{
    ASSERT(scene.lightCount < ARRAY_COUNT(scene.lights), "Reached max number of lights");
    scene.lights[scene.lightCount++] = light;
}

void AddDirectionalLight(Scene& scene, const vec3& color, const vec3& direction)
{
    Light light = {};
    light.type = LightType_Directional;
    light.color = color;
    light.direction = direction;
    AddLight(scene, light);
}

void AddPointLight(Scene& scene, const vec3& color, const vec3& position)
{
    Light light = {};
    light.type = LightType_Point;
    light.color = color;
    light.position = position;
    AddLight(scene, light);
}

mat4 TransformScale(const vec3& scaleFactors)
{
    mat4 transform = scale(scaleFactors);
    return transform;
}

mat4 TransformPositionScale(const vec3& pos, const vec3& scaleFactors)
{
    mat4 transform = translate(pos);
    transform = scale(transform, scaleFactors);
    return transform;
}

void DebugDraw_Clear(DebugDraw& debugDraw)
{
    debugDraw.opaqueLineCount = 0;
    debugDraw.texQuadCount = 0;
}

void DebugDrawLine(Device& device, DebugDraw& debugDraw, const vec3& p1, const vec3& p2, const vec3& color)
{
    struct DebugLineVertex { vec3 pos; vec3 col; };
    DebugLineVertex vertex1 = { p1, color };
    DebugLineVertex vertex2 = { p2, color };

    Buffer& vertexBuffer = device.vertexBuffers[debugDraw.opaqueLineVertexBufferIdx];
    PushAlignedData(vertexBuffer, &vertex1, sizeof(DebugLineVertex), 1);
    PushAlignedData(vertexBuffer, &vertex2, sizeof(DebugLineVertex), 1);
    debugDraw.opaqueLineCount++;
}

#if USE_GFX_API_OPENGL
void DebugDrawTexturedQuad(DebugDraw& debugDraw, GLuint textureHandle, const vec4& rect)
{
    const u32 texQuadIdx = debugDraw.texQuadCount++;
    debugDraw.texQuadTextureHandles[texQuadIdx] = textureHandle;
    debugDraw.texQuadRects[texQuadIdx] = rect;
}
#endif

void InitDevice(Device& device)
{
    // First object is considered null
    device.textureCount = 1;
    device.materialCount = 1;
    device.meshCount = 1;
    device.programCount = 1;
    device.constantBufferCount = 1;
    device.renderTargetCount = 1;
    device.framebufferCount = 1;
    device.renderPassCount = 1;

#if USE_GFX_API_METAL
    Metal_InitDevice(device);
#elif USE_GFX_API_OPENGL
    OGL_InitDevice(device);
#endif
}

void InitEmbedded(Device& device, Embedded& embed)
{
    // Embedded geometry
    embed.meshIdx = device.meshCount++;
    Mesh& mesh = device.meshes[embed.meshIdx];
    mesh = Mesh{};

    mesh.vertexBufferIdx = CreateStaticVertexBuffer(device, MB(1));
    mesh.indexBufferIdx = CreateStaticIndexBuffer(device, MB(1));

    Buffer& vertexBuffer = device.vertexBuffers[mesh.vertexBufferIdx];
    Buffer& indexBuffer = device.indexBuffers[mesh.indexBufferIdx];
    
    MapBuffer(vertexBuffer, Access_Write);
    MapBuffer(indexBuffer, Access_Write);

    // Screen-filling triangle
    {
        struct VertexV3V2
        {
            vec3 pos;
            vec2 uv;
        };

        const VertexV3V2 vertices[] = {
            { vec3(-1.0, -1.0, 0.0), vec2(0.0, 0.0) }, // bottom-left vertex
            { vec3( 3.0, -1.0, 0.0), vec2(2.0, 0.0) }, // bottom-right vertex
            { vec3(-1.0,  3.0, 0.0), vec2(0.0, 2.0) }, // top-left vertex
        };

        const u32 indices[] = { 0, 1, 2 };

        embed.blitSubmeshIdx = mesh.submeshes.size();
        mesh.submeshes.push_back(Submesh{});
        Submesh& submesh = mesh.submeshes.back();
        submesh.vertexOffset = vertexBuffer.head;
        submesh.indexOffset = indexBuffer.head;
        submesh.vertexCount = ARRAY_COUNT(vertices);
        submesh.indexCount = ARRAY_COUNT(indices);
        submesh.vertexBufferLayout.stride = sizeof(VertexV3V2);
        submesh.vertexBufferLayout.attributes[0] = VertexBufferAttribute{0, 3, 0};
        submesh.vertexBufferLayout.attributes[1] = VertexBufferAttribute{2, 2, sizeof(vec3)};
        submesh.vertexBufferLayout.attributeCount = 2;

        BufferPushData(vertexBuffer, vertices, sizeof(vertices));
        BufferPushData(indexBuffer, indices, sizeof(indices));
    }

    // Floor plane
    {
        struct VertexV3V3V2
        {
            vec3 pos;
            vec3 nor;
            vec2 uv;
        };

        const VertexV3V3V2 vertices[] = {
            { vec3(-1.0, 0.0,  1.0), vec3(0.0, 1.0, 0.0), vec2(0.0, 0.0) }, // bottom-left vertex
            { vec3( 1.0, 0.0,  1.0), vec3(0.0, 1.0, 0.0), vec2(1.0, 0.0) }, // bottom-right vertex
            { vec3( 1.0, 0.0, -1.0), vec3(0.0, 1.0, 0.0), vec2(1.0, 1.0) }, // top-right vertex
            { vec3(-1.0, 0.0, -1.0), vec3(0.0, 1.0, 0.0), vec2(0.0, 1.0) }, // top-left vertex
        };

        const u32 indices[] = {
            0, 1, 2,
            0, 2, 3
        };

        embed.floorSubmeshIdx = mesh.submeshes.size();
        mesh.submeshes.push_back(Submesh{});
        Submesh& submesh = mesh.submeshes.back();
        submesh.vertexOffset = vertexBuffer.head;
        submesh.indexOffset = indexBuffer.head;
        submesh.vertexCount = ARRAY_COUNT(vertices);
        submesh.indexCount = ARRAY_COUNT(indices);
        submesh.vertexBufferLayout.stride = sizeof(VertexV3V3V2);
        submesh.vertexBufferLayout.attributes[0] = VertexBufferAttribute{0, 3, 0};
        submesh.vertexBufferLayout.attributes[1] = VertexBufferAttribute{1, 3, sizeof(vec3)};
        submesh.vertexBufferLayout.attributes[2] = VertexBufferAttribute{2, 2, 2*sizeof(vec3)};
        submesh.vertexBufferLayout.attributeCount = 3;

        BufferPushData(vertexBuffer, vertices, sizeof(vertices));
        BufferPushData(indexBuffer, indices, sizeof(indices));
    }

    // Sphere
    {
        struct VertexV3V3V2
        {
            vec3 pos;
            vec3 nor;
            vec2 uv;
        };

        const u32 HMAX = 16;
        const u32 VMAX = 8;

        ScratchArena vertexArena;
        for (u32 h = 0; h < HMAX; ++h)
        {
            for (u32 v = 0; v < VMAX + 1; ++v)
            {
                const float yaw = 2.0f * PI * (float)h/(float)HMAX;
                const float pitch = PI * (float)v/(float)VMAX;
                const vec3 pos( sinf(pitch) * sinf(yaw),
                                cosf(pitch),
                                sinf(pitch) * cosf(yaw) );
                VertexV3V3V2 vertex{pos, normalize(pos), vec2(0.0f, 0.0f)};
                PUSH_LVALUE(vertexArena, vertex);
            }
        }

        ScratchArena indexArena;
        for (u32 h = 0; h < HMAX; ++h)
        {
            for (u32 v = 0; v < VMAX; ++v)
            {
                const u32 a = h * (VMAX+1) + v;
                const u32 b = h * (VMAX+1) + v + 1;
                const u32 c = ((h+1)%HMAX) * (VMAX+1) + v + 1;
                const u32 d = ((h+1)%HMAX) * (VMAX+1) + v;
                PushU32(indexArena, a);
                PushU32(indexArena, b);
                PushU32(indexArena, c);
                PushU32(indexArena, a);
                PushU32(indexArena, c);
                PushU32(indexArena, d);
            }
        }

        embed.sphereSubmeshIdx = mesh.submeshes.size();
        mesh.submeshes.push_back(Submesh{});
        Submesh& submesh = mesh.submeshes.back();
        submesh.vertexOffset = vertexBuffer.head;
        submesh.indexOffset = indexBuffer.head;
        submesh.indexCount = indexArena.head / sizeof(u32);
        submesh.vertexCount = vertexArena.head / sizeof(VertexV3V3V2);
        submesh.vertexBufferLayout.stride = sizeof(VertexV3V3V2);
        submesh.vertexBufferLayout.attributes[0] = VertexBufferAttribute{0, 3, 0};
        submesh.vertexBufferLayout.attributes[1] = VertexBufferAttribute{1, 3, sizeof(vec3)};
        submesh.vertexBufferLayout.attributes[2] = VertexBufferAttribute{2, 2, 2*sizeof(vec3)};
        submesh.vertexBufferLayout.attributeCount = 3;

        BufferPushData(vertexBuffer, vertexArena.data, vertexArena.head);
        BufferPushData(indexBuffer, indexArena.data, indexArena.head);
    }

    UnmapBuffer(vertexBuffer);
    UnmapBuffer(indexBuffer);

    // Textures
    embed.diceTexIdx = LoadTexture2D(device, "dice.png");
    embed.whiteTexIdx = LoadTexture2D(device, "color_white.png");
    embed.blackTexIdx = LoadTexture2D(device, "color_black.png");
    embed.normalTexIdx = LoadTexture2D(device, "color_normal.png");
    embed.magentaTexIdx = LoadTexture2D(device, "color_magenta.png");

    // Materials
    Material defaultMaterial = {};
    defaultMaterial.name = CString("defaultMaterial");
    defaultMaterial.albedo = vec3(1.0);
    defaultMaterial.emissive = vec3(0.0);
    defaultMaterial.smoothness = 0.0;
    defaultMaterial.albedoTextureIdx = embed.whiteTexIdx;
    defaultMaterial.emissiveTextureIdx = embed.blackTexIdx;
    defaultMaterial.specularTextureIdx = embed.blackTexIdx;
    defaultMaterial.normalsTextureIdx = embed.normalTexIdx;
    defaultMaterial.bumpTextureIdx = embed.blackTexIdx;
    embed.defaultMaterialIdx = device.materialCount;
    device.materials[device.materialCount++] = defaultMaterial;

    // Textured geometry program
    embed.texturedGeometryProgramIdx = LoadProgram(device, CString("shaders.glsl"), CString("TEXTURED_GEOMETRY"));
    Program& texturedGeometryProgram = device.programs[embed.texturedGeometryProgramIdx];
#if USE_GFX_API_OPENGL
    embed.texturedGeometryProgram_TextureLoc = glGetUniformLocation(texturedGeometryProgram.handle, "uTexture");
#endif
}

void InitDebugDraw(Device& device, DebugDraw& debugDraw)
{
    debugDraw.opaqueProgramIdx = LoadProgram(device, CString("shaders.glsl"), CString("DEBUG_DRAW_OPAQUE"));
    debugDraw.opaqueLineVertexBufferIdx = CreateDynamicVertexBuffer(device, KB(256));
    debugDraw.opaqueLineCount = 0;
    u32 vertexBufferOffset = 0;
    VertexBufferLayout vertexBufferLayout = {};
    vertexBufferLayout.attributes[0] = VertexBufferAttribute{ 0, 3, 0 };
    vertexBufferLayout.attributes[1] = VertexBufferAttribute{ 5, 3, sizeof(vec3) };
    vertexBufferLayout.attributeCount = 2;
    vertexBufferLayout.stride = 2 * sizeof(vec3);
    Program program = device.programs[debugDraw.opaqueProgramIdx];
    Buffer& vertexBuffer = device.vertexBuffers[debugDraw.opaqueLineVertexBufferIdx];
    debugDraw.opaqueLineVao = CreateVAORaw(vertexBuffer,
                                           vertexBufferOffset,
                                           vertexBufferLayout,
                                           program.vertexInputLayout,
                                           program, 0, 0);
}

void InitScene(Device& device, Scene& scene, Embedded& embedded)
{
    // Models
    scene.patrickModelIdx = LoadModel(device, "Patrick/Patrick.obj");

    // Camera
    Camera& camera = scene.mainCamera;
    camera.yaw = -0.7f;
    camera.pitch = -0.3f;
    camera.position = vec3(7.0, 4.0, 7.0);

    // Model/mesh entities
    AddMeshEntity(scene, embedded.meshIdx, embedded.floorSubmeshIdx, TransformScale(vec3(100.0f)));
    AddMeshEntity(scene, embedded.meshIdx, embedded.sphereSubmeshIdx, TransformScale(vec3(2.0f)));
    const u32 ENTITY_MULTIPLIER = 10;
    const f32 ENTITY_SEPARATION = 3.0f;
    for ( u32 i = 0; i < ENTITY_MULTIPLIER; ++i )
    {
        for ( u32 j = 0; j < ENTITY_MULTIPLIER; ++j )
        {
            f32 x = ENTITY_SEPARATION * (f32)i - 0.5f * ENTITY_MULTIPLIER * ENTITY_SEPARATION;
            f32 z = ENTITY_SEPARATION * (f32)j - 0.5f * ENTITY_MULTIPLIER * ENTITY_SEPARATION;
            AddModelEntity( scene, scene.patrickModelIdx, TransformPositionScale( vec3( x, 1.5f, z ), vec3( 0.45f ) ) );
        }
    }

    // Lights
    AddDirectionalLight( scene, vec3( 0.8, 0.8, 0.8 ), normalize( vec3( 1.0, 1.0, 1.0 ) ) );
    AddPointLight(scene, vec3(2.0, 1.5, 0.5), vec3( 0.0, 0.5, -4.0));
    AddPointLight(scene, vec3(2.0, 1.5, 0.5), vec3( 4.0, 0.5,  3.0));
    AddPointLight(scene, vec3(2.0, 1.5, 0.5), vec3(-4.0, 0.5,  3.0));
}

#include "renderers.cpp"

void Init(App* app)
{
    gApp = app;

    StrArena = CreateArena(MB(1));

    Device& device = app->device;

    InitDevice(device);

    InitEmbedded(device, app->embedded);

#if USE_GFX_API_METAL
    return;
#endif

    InitDebugDraw(device, app->debugDraw);

    ForwardShading_Init(device, app->forwardRenderData);

    DeferredShading_Init(device, app->deferredRenderData);

    InitScene(device, app->scene, app->embedded);

    app->globalParamsBlockSize = KB(1); // TODO: Get the size from the shader?

    // Render targets
    app->albedoRenderTargetIdx = CreateRenderTarget(device, CString("Albedo"), RenderTargetType_Color, app->displaySize);
    app->normalRenderTargetIdx = CreateRenderTarget(device, CString("Normal"), RenderTargetType_Color, app->displaySize);
    app->positionRenderTargetIdx = CreateRenderTarget(device, CString("Position"), RenderTargetType_Floats, app->displaySize);
    app->radianceRenderTargetIdx = CreateRenderTarget(device, CString("Radiance"), RenderTargetType_Color, app->displaySize);
    app->depthRenderTargetIdx = CreateRenderTarget(device, CString("Depth"), RenderTargetType_Depth, app->displaySize);

    // Framebuffers
    {
        Attachment attachments[] = {
            {Attachment_Color0, app->albedoRenderTargetIdx,  }, 
            {Attachment_Color1, app->normalRenderTargetIdx,  }, 
            {Attachment_Color2, app->positionRenderTargetIdx,}, 
            {Attachment_Color3, app->radianceRenderTargetIdx,},
            {Attachment_Depth,  app->depthRenderTargetIdx,   }, 
        };
        app->gbufferFramebufferIdx = CreateFramebuffer(device, ARRAY_COUNT(attachments), attachments);
    }
    {
        Attachment attachments[] = {
            {Attachment_Color0, app->radianceRenderTargetIdx, },
            {Attachment_Depth,  app->depthRenderTargetIdx,    },
        };
        app->forwardFramebufferIdx = CreateFramebuffer(device, ARRAY_COUNT(attachments), attachments);
    }

    // Render passes
    {
        AttachmentAction attachments[] = {
            {0, LoadOp_Clear, StoreOp_Store},
            {1, LoadOp_Clear, StoreOp_Store},
            {2, LoadOp_Clear, StoreOp_Store},
            {4, LoadOp_Clear, StoreOp_Store},
        };
        app->gbufferPassIdx = CreateRenderPass(device, app->gbufferFramebufferIdx, ARRAY_COUNT(attachments), attachments);
    }
    {
        AttachmentAction attachments[] = {
            {3, LoadOp_Clear, StoreOp_Store},
            {4, LoadOp_Load,  StoreOp_DontCare},
        };
        app->deferredShadingPassIdx = CreateRenderPass(device, app->gbufferFramebufferIdx, ARRAY_COUNT(attachments), attachments);
    }
    {
        AttachmentAction attachments[] = {
            {0, LoadOp_Clear, StoreOp_Store},
            {1, LoadOp_Clear,  StoreOp_DontCare},
        };
        app->forwardShadingPassIdx = CreateRenderPass(device, app->forwardFramebufferIdx, ARRAY_COUNT(attachments), attachments);
    }



    app->frameRenderGroup = RegisterRenderGroup(app, "Frame");

    ProfileEvent_Init(app);

    app->renderPath = RenderPath_ForwardShading;
}

void DebugDraw_Render(Device& device, Embedded& embedded, DebugDraw& debugDraw, BufferRange& globalParams)
{
#if USE_GFX_API_OPENGL
    if (debugDraw.opaqueLineCount > 0)
    {
        RENDER_GROUP("Debug draw - opaque lines", gApp->frameRenderGroup);

        const Program& program = device.programs[debugDraw.opaqueProgramIdx];
        glUseProgram(program.handle);

        glBindBufferRange(GL_UNIFORM_BUFFER, BINDING(0), device.constantBuffers[globalParams.bufferIdx].handle, globalParams.offset, globalParams.size);

        glBindVertexArray(debugDraw.opaqueLineVao.handle);

        glDrawArrays(GL_LINES, 0, debugDraw.opaqueLineCount * 2);
    }

    if (debugDraw.texQuadCount > 0)
    {
        RENDER_GROUP("Debug draw - textured quads", gApp->frameRenderGroup);

        Program& program = device.programs[embedded.texturedGeometryProgramIdx];
        glUseProgram(program.handle);

        GLuint vaoHandle = FindVAO(device, embedded.meshIdx, embedded.blitSubmeshIdx, program);
        glBindVertexArray(vaoHandle);

        glDisable(GL_DEPTH_TEST);
        glDisable(GL_BLEND);

        for (u32 i = 0; i < debugDraw.texQuadCount; ++i)
        {
            ivec4 viewportRect = debugDraw.texQuadRects[i];
            glViewport(viewportRect.x, viewportRect.y, viewportRect.z, viewportRect.w);

            glActiveTexture(GL_TEXTURE0);
            GLuint textureHandle = debugDraw.texQuadTextureHandles[i];
            glBindTexture(GL_TEXTURE_2D, textureHandle);
            glUniform1i(embedded.texturedGeometryProgram_TextureLoc, 0);

            glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, 0);
        }

        glBindVertexArray(0);
        glUseProgram(0);
    }
#endif
}

void BeginFrame(App* app)
{
#if USE_GFX_API_METAL
    Metal_BeginFrame();
    return;
#endif

    app->frame++;
    app->frameMod = app->frame % MAX_GPU_FRAME_DELAY;

    ProfileEvent_Insert(app, app->frameRenderGroup, ProfileEventType_FrameBegin);
}

void Gui(App* app)
{
#if USE_GFX_API_METAL
    ImGui::Begin("Info");
    ImGui::Text("Device name: Unknown");
    ImGui::Text("OGL Version: Unknown");
    ImGui::Text("FPS: ---");
    ImGui::End();
    return;
#endif

    DebugDraw_Clear(app->debugDraw);

    Device& device = app->device;

    ImGui::Begin("Info");
    ImGui::Text("Device name: %s", device.name);
    ImGui::Text("OGL Version: %s", device.glVersionString);
    ImGui::Text("FPS: %f", 1.0f/app->deltaTime);

    ImGui::Separator();
    
    ImGui::Text("Camera");
    float yawPitch[3] = {360.0f * app->scene.mainCamera.yaw / TAU, 360.0f * app->scene.mainCamera.pitch / TAU};
    ImGui::InputFloat3("Yaw/Pitch/Roll", yawPitch, "%.3f", ImGuiInputTextFlags_ReadOnly);
    ImGui::InputFloat3("Position", value_ptr(app->scene.mainCamera.position), "%.3f", ImGuiInputTextFlags_ReadOnly);
    
    ImGui::Separator();

    ImGui::Checkbox("Back-face culling", &g_CullFace);

    if (ImGui::Button("Take snapshot"))
    {
        app->takeSnapshot = true;
    }

    ImGui::Separator();

//    for (u32 renderGroupIdx = 0; renderGroupIdx < app->renderGroupCount; ++renderGroupIdx)
//    {
//        f32 timeMs = 0.0f;
//        if (app->frame >= MAX_GPU_FRAME_DELAY )
//        {
////#if defined(TIMESTAMP_QUERIES)
//            const GLuint beginTimerQuery = app->timerQueries[ GetTimerQueryIndex(app->frameMod, renderGroupIdx, 0) ];
//            const GLuint endTimerQuery   = app->timerQueries[ GetTimerQueryIndex(app->frameMod, renderGroupIdx, 1) ];
//            GLint endTimerQueryAvailable = 0;
//            while (!endTimerQueryAvailable)
//                glGetQueryObjectiv(endTimerQuery, GL_QUERY_RESULT_AVAILABLE, &endTimerQueryAvailable);
//            GLuint64 beginTimeNs;
//            GLuint64 endTimeNs;
//            glGetQueryObjectui64v(beginTimerQuery, GL_QUERY_RESULT, &beginTimeNs);
//            glGetQueryObjectui64v(endTimerQuery,   GL_QUERY_RESULT, &endTimeNs);
//            timeMs = (endTimeNs - beginTimeNs) / 1000000.0f;
////#else
////            if (renderGroupIdx == 0 ) continue;
////            const GLuint timeElapsedQuery = app->timerQueries[ GetTimerQueryIndex(app->frameMod, renderGroupIdx, 0) ];
////            // Wait for all results to become available
////            GLint available = 0;
////            while (!available) glGetQueryObjectiv(timeElapsedQuery, GL_QUERY_RESULT_AVAILABLE, &available);
////            GLuint64 timeElapsedNs;
////            glGetQueryObjectui64v(timeElapsedQuery, GL_QUERY_RESULT, &timeElapsedNs);
////            timeMs = timeElapsedNs / 1000000.0f;
////#endif
//        }
//        char buf[128];
//        sprintf(buf, "%.03f (ms)", timeMs);
//        ImGui::PushStyleVar(ImGuiStyleVar_ButtonTextAlign, ImVec2(1.0, 0.5));
//        ImGui::ProgressBar(timeMs/16.0f, ImVec2(0.0f, 0.0f), buf);
//        ImGui::PopStyleVar();
//        ImGui::SameLine();
//        ImGui::Text("%s", app->renderGroups[renderGroupIdx].name);
//    }

#if USE_GFX_API_OPENGL
    if (app->frame >= MAX_GPU_FRAME_DELAY)
    {
        ASSERT(app->profileEventCount > 0, "No profile events... not event BeginFrame?");
        ASSERT(app->profileEventTypes[app->profileEventBegin] == ProfileEventType_FrameBegin, "First profile event should be FrameBegin...");

        // We start right after the first profile event at BeginFrame
        u32 profileGroupCount = 1;
        u32 openProfileGroupCount = 1;
        u32 openProfileGroupStack[8] = {};
        openProfileGroupStack[0] = app->profileEventBegin;
        app->profileEventBegin = (app->profileEventBegin + 1) % MAX_PROFILE_EVENTS;
        app->profileEventCount--;

        float renderGroupTimes[MAX_RENDER_GROUPS] = {};

        while (openProfileGroupCount > 0)
        {
            ASSERT(app->profileEventCount > 0, "No more profile events... maybe there was a mismatch (more Begin than End)");

            if (app->profileEventTypes[app->profileEventBegin] == ProfileEventType_GroupBegin)
            {
                ASSERT(openProfileGroupCount < ARRAY_COUNT(openProfileGroupStack), "Too many levels of profile groups");
                openProfileGroupStack[openProfileGroupCount] = app->profileEventBegin;
                profileGroupCount++;
                openProfileGroupCount++;
            }
            else if (app->profileEventTypes[app->profileEventBegin] == ProfileEventType_GroupEnd ||
                     app->profileEventTypes[app->profileEventBegin] == ProfileEventType_FrameEnd )
            {
                openProfileGroupCount--;

#if TIMESTAMP_QUERIES
                const u32 beginTimerQueryIdx = openProfileGroupStack[openProfileGroupCount];
                const u32 endTimerQueryIdx = app->profileEventBegin;

                const GLuint beginTimerQuery = app->profileEventQueries[ beginTimerQueryIdx ];
                const GLuint endTimerQuery   = app->profileEventQueries[ endTimerQueryIdx ];
                GLint endTimerQueryAvailable = 0;
                while (!endTimerQueryAvailable)
                    glGetQueryObjectiv(endTimerQuery, GL_QUERY_RESULT_AVAILABLE, &endTimerQueryAvailable);
                GLuint64 beginTimeNs;
                GLuint64 endTimeNs;
                glGetQueryObjectui64v(beginTimerQuery, GL_QUERY_RESULT, &beginTimeNs);
                glGetQueryObjectui64v(endTimerQuery,   GL_QUERY_RESULT, &endTimeNs);
                const float timeMs = (endTimeNs - beginTimeNs) / 1000000.0f;
#else
                // TODO
                const float timeMs = 0.0f;
#endif

                const u32 renderGroupIdx = app->profileEventGroup[ app->profileEventBegin ];
                renderGroupTimes[renderGroupIdx] += timeMs;
            }
            else
            {
                INVALID_CODE_PATH("Unsupported ProfileEventType");
            }

            app->profileEventBegin = (app->profileEventBegin + 1) % MAX_PROFILE_EVENTS;
            app->profileEventCount--;
        }

        for (u32 renderGroupIdx = 0; renderGroupIdx < app->renderGroupCount; ++renderGroupIdx)
        {
            char buf[128];
            const float timeMs = renderGroupTimes[renderGroupIdx];
            sprintf(buf, "%.03f (ms)", timeMs);
            ImGui::PushStyleVar(ImGuiStyleVar_ButtonTextAlign, ImVec2(1.0, 0.5));
            ImGui::ProgressBar(timeMs/16.0f, ImVec2(0.0f, 0.0f), buf);
            ImGui::PopStyleVar();
            ImGui::SameLine();
            ImGui::Text("%s", app->renderGroups[renderGroupIdx].name);
        }
    }
#endif

    ImGui::Separator();

    const char* renderPathNames[] = {"Forward", "Deferred"};
    ASSERT(ARRAY_COUNT(renderPathNames) == RenderPath_Count, "Number of render paths do not match");
    const char* currentItem = renderPathNames[app->renderPath];
    if (ImGui::BeginCombo("Render path", currentItem))
    {
        for (u32 renderPathIdx = 0; renderPathIdx < ARRAY_COUNT(renderPathNames); ++renderPathIdx)
        {
            bool isSelected = app->renderPath == renderPathIdx;
            if (ImGui::Selectable(renderPathNames[renderPathIdx], isSelected))
                app->renderPath = (RenderPath)renderPathIdx;
            if (isSelected)
                ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }

#if USE_GFX_API_OPENGL
    ImGui::Separator();

    static u32    visibleTextureCount = 0;
    static GLuint visibleTextures[16];

    u32    newVisibleTexturesCount = 0;
    GLuint newVisibleTextures[16];

    u32 yOffset = 0;

    if (ImGui::CollapsingHeader("Textures"))
    {
        for (u32 i = 1; i < app->device.textureCount; ++i)
        {
            const Texture& texture = app->device.textures[i];

            bool showInDebugDraw = false;
            for (u32 i = 0; i < visibleTextureCount && !showInDebugDraw; ++i)
                showInDebugDraw = (visibleTextures[i] == texture.handle);

            ImGui::Checkbox(texture.filepath.str, &showInDebugDraw);

            if (showInDebugDraw)
            {
                ASSERT(newVisibleTexturesCount < ARRAY_COUNT(newVisibleTextures), "Debug textured quads limit reached");
                newVisibleTextures[newVisibleTexturesCount++] = texture.handle;

                const ivec4 rect(0, yOffset, 64, 64);
#if USE_GFX_API_OPENGL
                DebugDrawTexturedQuad(app->debugDraw, texture.handle, rect);
#endif
                yOffset += 64;
            }
        }
    }

    if (ImGui::CollapsingHeader("Render targets"))
    {
        for (u32 i = 1; i < device.renderTargetCount; ++i)
        {
            const RenderTarget& renderTarget = app->device.renderTargets[i];

            bool showInDebugDraw = false;
            for (u32 i = 0; i < visibleTextureCount && !showInDebugDraw; ++i)
                showInDebugDraw = (visibleTextures[i] == renderTarget.handle);

            ImGui::Checkbox(renderTarget.name.str, &showInDebugDraw);

            if (showInDebugDraw)
            {
                ASSERT(newVisibleTexturesCount < ARRAY_COUNT(newVisibleTextures), "Debug textured quads limit reached");
                newVisibleTextures[newVisibleTexturesCount++] = renderTarget.handle;

                const ivec4 rect(0, yOffset, 64, 64);
#if USE_GFX_API_OPENGL
                DebugDrawTexturedQuad(app->debugDraw, renderTarget.handle, rect);
#endif
                yOffset += 64;
            }
        }
    }

    visibleTextureCount = newVisibleTexturesCount;
    for (u32 i = 0; i < visibleTextureCount; ++i)
        visibleTextures[i] = newVisibleTextures[i];
#endif

    ImGui::End();

    //ImGui::ShowDemoWindow();
}

void Resize(App* app)
{
#if USE_GFX_API_METAL
    return;
#endif

    Device& device = app->device;

    // Resize render targets
    for (u32 i = 1; i < device.renderTargetCount; ++i)
    {
        RenderTarget& renderTarget = device.renderTargets[i];
        DestroyRenderTargetRaw(renderTarget);
        renderTarget = CreateRenderTargetRaw(renderTarget.name, app->displaySize, renderTarget.type);
    }

    // Recreate framebuffers
    for (u32 i = 1; i < device.framebufferCount; ++i)
    {
        Framebuffer& framebuffer = device.framebuffers[i];
        DestroyFramebufferRaw(framebuffer);
        framebuffer = CreateFramebufferRaw(device, framebuffer.attachmentCount, framebuffer.attachments);
    }
}

void Update(App* app)
{
#if USE_GFX_API_METAL
    return;
#endif

    if (app->input.mouseButtons[LEFT] == BUTTON_PRESS)
        ILOG("Mouse button left pressed");

    if (app->input.mouseButtons[LEFT] == BUTTON_RELEASE)
        ILOG("Mouse button left released");

    for (u64 i = 0; i < app->device.programCount; ++i)
    {
        Program& program = app->device.programs[i];
        u64 currentTimestamp = GetFileLastWriteTimestamp(program.filepath.str);
        if (currentTimestamp > program.lastWriteTimestamp)
        {
            String programSource = ReadTextFile(program.filepath.str);
            const char* programName = program.programName.str;
#if USE_GFX_API_OPENGL
            glDeleteProgram(program.handle);
            program.handle = CreateProgramFromSource(programSource, app->device.glslVersion, programName);
            program.vertexInputLayout = ExtractVertexShaderLayoutFromProgram(program.handle);
#endif
            program.lastWriteTimestamp = currentTimestamp;
        }
    }

    // Update camera
    Camera& camera = app->scene.mainCamera;

    const float rotationSpeed = 0.1f * PI;
    if (app->input.mouseButtons[RIGHT] == BUTTON_PRESSED)
    {
        camera.yaw += app->input.mouseDelta.x * rotationSpeed * app->deltaTime;
        camera.pitch -= app->input.mouseDelta.y * rotationSpeed * app->deltaTime;
    }
    camera.yaw = mod(camera.yaw, TAU);
    camera.pitch = clamp(camera.pitch, -PI/2.1f, PI/2.1f);
    camera.forward = vec3(cosf(camera.pitch)*sinf(camera.yaw),
                               sinf(camera.pitch),
                               -cosf(camera.pitch)*cosf(camera.yaw));
    camera.right = vec3(cosf(camera.yaw), 0.0f, sinf(camera.yaw));
    vec3 upVector = vec3(0.0f, 1.0f, 0.0f);

    vec3 newDirection = vec3(0.0);
    if (app->input.keys[K_W] == BUTTON_PRESSED) { newDirection += camera.forward; }
    if (app->input.keys[K_S] == BUTTON_PRESSED) { newDirection -= camera.forward; }
    if (app->input.keys[K_D] == BUTTON_PRESSED) { newDirection += camera.right;   }
    if (app->input.keys[K_A] == BUTTON_PRESSED) { newDirection -= camera.right;   }

    const float newdirMagnitude = length(newDirection);
    newDirection = (newdirMagnitude > 0.0f) ? newDirection / newdirMagnitude : vec3(0.0f);

    float speedMagnitude = length(camera.speed);
    vec3 speedDirection = (speedMagnitude > 0.0f) ? camera.speed / speedMagnitude : vec3(0.0f);

    const float MAX_SPEED = 100.0f;
    if (newdirMagnitude > 0.0f) {
        speedDirection = 0.5f * (speedDirection + newDirection);
        speedMagnitude = min(speedMagnitude + 1.0f, MAX_SPEED);
    } else {
        speedMagnitude *= 0.8f;
        if (speedMagnitude < 0.01f)
            speedMagnitude = 0.0f;
    }

    camera.speed = speedMagnitude * speedDirection;

    camera.position += camera.speed * app->deltaTime;

    float aspectRatio = (float)app->displaySize.x/(float)app->displaySize.y;
    camera.viewMatrix = lookAt(camera.position, camera.position + camera.forward, upVector);
    camera.projectionMatrix = perspective(radians(60.0f), aspectRatio, 0.1f, 1000.0f);
    camera.viewProjectionMatrix = camera.projectionMatrix * camera.viewMatrix;

    // Upload uniforms to buffer

    BeginConstantBufferRecording( app->device );

    Buffer& constantBuffer = GetMappedConstantBufferForRange( app->device, app->globalParamsBlockSize );

    // -- Global params
    app->globalParamsBufferIdx = app->device.currentConstantBufferIdx;
    app->globalParamsOffset = constantBuffer.head;

    BufferPushMat4(constantBuffer, camera.viewProjectionMatrix);
    BufferPushVec3(constantBuffer, camera.position);

    BufferPushUInt(constantBuffer, app->scene.lightCount);

    for (u32 i = 0; i < app->scene.lightCount; ++i)
    {
        AlignHead(constantBuffer, sizeof(vec4));

        Light& light = app->scene.lights[i];
        BufferPushUInt(constantBuffer, light.type);
        BufferPushVec3(constantBuffer, light.color);
        BufferPushVec3(constantBuffer, light.direction);
        BufferPushVec3(constantBuffer, light.position);
    }

    app->globalParamsSize = constantBuffer.head - app->globalParamsOffset;

    switch (app->renderPath)
    {
        case RenderPath_ForwardShading:
            ForwardShading_Update(app->device, app->scene, app->embedded, app->forwardRenderData);
            break;
        case RenderPath_DeferredShading:
            DeferredShading_Update(app->device, app->scene, app->embedded, app->deferredRenderData);
            break;
        default:
            ASSERT(0, "Invalid code path");
    }

    EndConstantBufferRecording( app->device );

#if 0
    // Some debug drawing
    Buffer& vertexBuffer = app->device.vertexBuffers[app->debugDraw.opaqueLineVertexBufferIdx];
    MapBuffer(vertexBuffer, Access_Write);
    for (u32 i = 0; i < app->scene.entityCount; ++i)
    {
        Entity& entity = app->scene.entities[i];
        DebugDrawLine(app->device, app->debugDraw, entity.worldMatrix[3], vec3(entity.worldMatrix[3]) + vec3(0.0, 5.0, 0.0), vec3(1.0, 0.0, 0.0));
    }
    UnmapBuffer(vertexBuffer);
#endif
}

#if USE_GFX_API_OPENGL
void BlitTexture(Device& device, const Embedded& embedded, ivec4 viewportRect, GLuint textureHandle)
{
    glViewport(viewportRect.x, viewportRect.y, viewportRect.z, viewportRect.w);

    Program& program = device.programs[embedded.texturedGeometryProgramIdx];
    glUseProgram(program.handle);

    GLuint vaoHandle = FindVAO(device, embedded.meshIdx, embedded.blitSubmeshIdx, program);
    glBindVertexArray(vaoHandle);

    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glUniform1i(embedded.texturedGeometryProgram_TextureLoc, 0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureHandle);

    glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, 0);

    glBindVertexArray(0);
    glUseProgram(0);
}
#endif

void Render(App* app)
{
#if USE_GFX_API_METAL
    Metal_Render(app);
    return;
#endif

#if USE_GFX_API_OPENGL
    Device& device = app->device;

    switch (app->renderPath)
    {
        case RenderPath_ForwardShading:
            {
                RENDER_GROUP("Forward render", gApp->frameRenderGroup);

                BeginRenderPass(device, app->forwardShadingPassIdx);

                glViewport(0.0f, 0.0f, app->displaySize.x, app->displaySize.y);
                glEnable(GL_DEPTH_TEST);

                BufferRange globalParamsRange = {
                    app->globalParamsBufferIdx,
                    app->globalParamsOffset,
                    app->globalParamsSize
                };

                ForwardShading_Render(app->device, app->embedded, app->forwardRenderData, globalParamsRange);

                DebugDraw_Render(device, app->embedded, app->debugDraw, globalParamsRange);

                glBindVertexArray(0);

                glUseProgram(0);

                EndRenderPass(device);
            }
            {
                RenderTarget& renderTarget = device.renderTargets[app->radianceRenderTargetIdx];
                ivec4 viewportRect(0, 0, app->displaySize.x, app->displaySize.y);
#if USE_GFX_API_OPENGL
                BlitTexture(app->device, app->embedded, viewportRect, renderTarget.handle);
#endif
            }
            break;

        case RenderPath_DeferredShading:
            {
                RENDER_GROUP("Deferred render", gApp->frameRenderGroup);

                BeginRenderPass(device, app->gbufferPassIdx);

                glViewport(0.0f, 0.0f, app->displaySize.x, app->displaySize.y);
                glEnable(GL_DEPTH_TEST);

                BufferRange globalParamsRange = {
                    app->globalParamsBufferIdx,
                    app->globalParamsOffset,
                    app->globalParamsSize
                };

                DeferredShading_RenderOpaques(app->device, app->embedded, app->deferredRenderData, globalParamsRange);
                EndRenderPass(device);

                BeginRenderPass(device, app->deferredShadingPassIdx);

                DeferredShading_RenderLights(app->device, app->embedded, app->deferredRenderData, globalParamsRange);

                DebugDraw_Render(device, app->embedded, app->debugDraw, globalParamsRange);

                EndRenderPass(device);

                glBindVertexArray(0);

                glUseProgram(0);

            }
            {
                RenderTarget& renderTarget = device.renderTargets[app->radianceRenderTargetIdx];
                ivec4 viewportRect(0, 0, app->displaySize.x, app->displaySize.y);
#if USE_GFX_API_OPENGL
                BlitTexture(app->device, app->embedded, viewportRect, renderTarget.handle);
#endif
            }
            break;

        default:;
    }
#endif

    //
    // Read pixels
    //

#if USE_GFX_API_OPENGL
    if (app->takeSnapshot)
    {
        glFinish();

        const u32 width = app->displaySize.x;
        const u32 height = app->displaySize.y;
        u8* outPixels = new u8[width*height*3];
        glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, outPixels);
        stbi_write_png("snapshot.png", width, height, 3, (const void*)outPixels, width*3);
        delete[] outPixels;
    }
#endif
}

void EndFrame(App* app)
{
#if USE_GFX_API_METAL
    Metal_EndFrame();
    return;
#endif

    ProfileEvent_Insert(app, app->frameRenderGroup, ProfileEventType_FrameEnd);
}

