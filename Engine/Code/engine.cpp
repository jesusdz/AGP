#include "engine.h"
#include <imgui.h>
#include <stb_image.h>
#include <stb_image_write.h>
#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <vector>

void OnGlError(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam)
{
    ELOG("OpenGL error: %s", message);
}

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
        (GLint) strlen(versionString),
        (GLint) strlen(shaderNameDefine),
        (GLint) strlen(vertexShaderDefine),
        (GLint) shaderSource.length
    };
    const GLchar* fragmentShaderSource[] = {
        versionString,
        shaderNameDefine,
        fragmentShaderDefine,
        shaderSource.str
    };
    const GLint fragmentShaderLengths[] = {
        (GLint) strlen(versionString),
        (GLint) strlen(shaderNameDefine),
        (GLint) strlen(fragmentShaderDefine),
        (GLint) shaderSource.length
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
    img.pixels = stbi_load(filename, &img.size.x, &img.size.y, &img.nchannels, 4);
    img.stride = img.size.x * img.nchannels;
    return img;
}

void FreeImage(Image image)
{
    stbi_image_free(image.pixels);
}

void ProcessMesh(aiMesh *mesh, const aiScene *scene, Mesh *myMesh, void **myMaterials, void **mySubmeshMaterials)
{
    std::vector<float> vertices;
    std::vector<u32> indices;

    bool hasTexCoords = false;
    bool hasTangentSpace = false;

    // process vertices
    for(unsigned int i = 0; i < mesh->mNumVertices; i++)
    {
        vertices.push_back(mesh->mVertices[i].x);
        vertices.push_back(mesh->mVertices[i].y);
        vertices.push_back(mesh->mVertices[i].z);
        vertices.push_back(mesh->mNormals[i].x);
        vertices.push_back(mesh->mNormals[i].y);
        vertices.push_back(mesh->mNormals[i].z);

        if(mesh->mTextureCoords[0]) // does the mesh contain texture coordinates?
        {
            hasTexCoords = true;
            vertices.push_back(mesh->mTextureCoords[0][i].x);
            vertices.push_back(mesh->mTextureCoords[0][i].y);
        }

        if(mesh->mTangents != nullptr && mesh->mBitangents)
        {
            hasTangentSpace = true;
            vertices.push_back(mesh->mTangents[i].x);
            vertices.push_back(mesh->mTangents[i].y);
            vertices.push_back(mesh->mTangents[i].z);

            // For some reason ASSIMP gives me the bitangents flipped.
            // Maybe it's my fault, but when I generate my own geometry
            // in other files (see the generation of standard assets)
            // and all the bitangents have the orientation I expect,
            // everything works ok.
            // I think that (even if the documentation says the opposite)
            // it returns a left-handed tangent space matrix.
            // SOLUTION: I invert the components of the bitangent here.
            vertices.push_back(-mesh->mBitangents[i].x);
            vertices.push_back(-mesh->mBitangents[i].y);
            vertices.push_back(-mesh->mBitangents[i].z);
        }
    }

    // process indices
    for(unsigned int i = 0; i < mesh->mNumFaces; i++)
    {
        aiFace face = mesh->mFaces[i];
        for(unsigned int j = 0; j < face.mNumIndices; j++)
        {
            indices.push_back(face.mIndices[j]);
        }
    }

    // store the proper (previously proceessed) material for this mesh
    if(mesh->mMaterialIndex >= 0 && mySubmeshMaterials != nullptr && myMaterials != nullptr)
    {
        mySubmeshMaterials[myMesh->submeshes.size()] = myMaterials[mesh->mMaterialIndex];
    }

    // create the vertex format
    VertexBufferFormat vertexFormat = {};
    vertexFormat.attributes.push_back( VertexBufferAttribute{ 0, 0, 3 } );
    vertexFormat.attributes.push_back( VertexBufferAttribute { 1, 3*sizeof(float), 3 } );
    vertexFormat.stride = 6 * sizeof(float);
    if (hasTexCoords)
    {
        vertexFormat.attributes.push_back( VertexBufferAttribute { 2, vertexFormat.stride, 2 } );
        vertexFormat.stride += 2 * sizeof(float);
    }
    if (hasTangentSpace)
    {
        vertexFormat.attributes.push_back( VertexBufferAttribute { 3, vertexFormat.stride, 3 } );
        vertexFormat.stride += 3 * sizeof(float);

        vertexFormat.attributes.push_back( VertexBufferAttribute { 4, vertexFormat.stride, 3 } );
        vertexFormat.stride += 3 * sizeof(float);
    }

    // add the submesh into the mesh
    Submesh submesh = {};
    submesh.vertexFormat = vertexFormat;
    submesh.vertices.swap(vertices);
    submesh.indices.swap(indices);
    myMesh->submeshes.push_back( submesh );
}

void ProcessNode(aiNode *node, const aiScene *scene, Mesh *myMesh, void **myMaterials, void **mySubmeshMaterials)
{
    // process all the node's meshes (if any)
    for(unsigned int i = 0; i < node->mNumMeshes; i++)
    {
        aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
        ProcessMesh(mesh, scene, myMesh, myMaterials, mySubmeshMaterials);
    }

    // then do the same for each of its children
    for(unsigned int i = 0; i < node->mNumChildren; i++)
    {
        ProcessNode(node->mChildren[i], scene, myMesh, myMaterials, mySubmeshMaterials);
    }
}

Mesh LoadMesh(const char* filename)
{
    Mesh mesh = {};

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
    }
    else
    {
        ProcessNode(scene->mRootNode, scene, &mesh, NULL, NULL);
    }

    aiReleaseImport(scene);

    u32 vertexBufferSize = 0;
    u32 indexBufferSize = 0;

    for (u32 i = 0; i < mesh.submeshes.size(); ++i)
    {
        vertexBufferSize += mesh.submeshes[i].vertices.size() * sizeof(float);
        indexBufferSize  += mesh.submeshes[i].indices.size()  * sizeof(u32);
    }

    glGenBuffers(1, &mesh.vertexBufferHandle);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vertexBufferHandle);
    glBufferData(GL_ARRAY_BUFFER, vertexBufferSize, NULL, GL_STATIC_DRAW);

    glGenBuffers(1, &mesh.indexBufferHandle);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.indexBufferHandle);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexBufferSize, NULL, GL_STATIC_DRAW);

    u32 indicesOffset = 0;
    u32 verticesOffset = 0;

    for (u32 i = 0; i < mesh.submeshes.size(); ++i)
    {
        const void* verticesData = mesh.submeshes[i].vertices.data();
        const u32   verticesSize = mesh.submeshes[i].vertices.size() * sizeof(float);
        glBufferSubData(GL_ARRAY_BUFFER, verticesOffset, verticesSize, verticesData);
        mesh.submeshes[i].vertexOffset = verticesOffset;
        verticesOffset += verticesSize;

        const void* indicesData = mesh.submeshes[i].indices.data();
        const u32   indicesSize = mesh.submeshes[i].indices.size() * sizeof(u32);
        glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, indicesOffset, indicesSize, indicesData);
        mesh.submeshes[i].indexOffset = indicesOffset;
        indicesOffset += indicesSize;
    }

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    return mesh;
}

Texture LoadTexture2D(Image image)
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

    Texture tex = {};
    glGenTextures(1, &tex.handle);
    glBindTexture(GL_TEXTURE_2D, tex.handle);
    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, image.size.x, image.size.y, 0, dataFormat, dataType, image.pixels);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glGenerateMipmap(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0);

    return tex;
}

GLuint FindVAO(Mesh& mesh, u32 submeshIndex, GLuint program)
{
    Submesh& submesh = mesh.submeshes[submeshIndex];

    // Try finding a vao for this submesh/program
    for (u32 i = 0; i < (u32)submesh.vaoInfos.size(); ++i)
        if (submesh.vaoInfos[i].program == program)
            return submesh.vaoInfos[i].vao;

    // Create a new vao for this submesh/program
    GLuint vao = 0;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glBindBuffer(GL_ARRAY_BUFFER, mesh.vertexBufferHandle);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.indexBufferHandle);

    for (u32 j = 0; j < submesh.vertexFormat.attributes.size(); ++j)
    {
        const u32 index  = submesh.vertexFormat.attributes[j].location;
        const u32 ncomp  = submesh.vertexFormat.attributes[j].componentCount;
        const u32 offset = submesh.vertexFormat.attributes[j].offset + submesh.vertexOffset; // attribute offset + vertex offset
        const u32 stride = submesh.vertexFormat.stride;
        glVertexAttribPointer(index, ncomp, GL_FLOAT, GL_FALSE, stride, (void*)(u64)offset);
        glEnableVertexAttribArray(index);
    }

    glBindVertexArray(0);

    // Store it in the list of vaos for this submesh
    VaoInfo vaoInfo = { vao, program };
    submesh.vaoInfos.push_back(vaoInfo);

    return vao;
}

void Init(App* app)
{
    if (GLVersion.major > 4 || (GLVersion.major == 4 && GLVersion.minor >= 3))
    {
        glDebugMessageCallback(OnGlError, app);
    }

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
    glGenBuffers(1, &app->embeddedGeometryVertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, app->embeddedGeometryVertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glGenBuffers(1, &app->embeddedGeometryIndexBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, app->embeddedGeometryIndexBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    // Attribute state
    glGenVertexArrays(1, &app->vao);
    glBindVertexArray(app->vao);
    glBindBuffer(GL_ARRAY_BUFFER, app->embeddedGeometryVertexBuffer);
    for (u32 i = 0; i < 5; ++i) // the program has attributes from 0 to 4
    {
        glVertexAttribBinding(i, 0);  // there will be only one buffer (at binding point 0) for all attributes
        glEnableVertexAttribArray(i);
    }
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, app->embeddedGeometryIndexBuffer);
    glBindVertexArray(0);

    String shaderSource = ReadTextFile("shaders.glsl");

    // Sprite pipeline
    app->program = LoadProgram(shaderSource, "TEXTURED_GEOMETRY");

    app->programUniformTexture = glGetUniformLocation(app->program, "uTexture");

    Image image = LoadImage("dice.png");
    app->tex = LoadTexture2D(image);
    FreeImage(image);

    // Mesh pipeline
    app->mesh = LoadMesh("Patrick/Patrick.obj");
    app->meshProgram = LoadProgram(shaderSource, "SHOW_MESH");

    FreeString(shaderSource);
}

void Gui(App* app)
{
    ImGui::Begin("Info");
    ImGui::Text("GPU Name: %s", app->gpuName);
    ImGui::Text("OGL Version: %s", app->openGlVersion);
    ImGui::Text("FPS: %f", 1.0f/app->deltaTime);
    ImGui::Separator();
    if (ImGui::Button("Take snapshot"))
    {
        app->takeSnapshot = true;
    }
    ImGui::End();
}

void Update(App* app)
{

}

void Render(App* app)
{
    //
    // Render pass: Draw cube texture
    //

#if 0
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glViewport(0, 0, app->displaySize.x, app->displaySize.y);

    glUseProgram(app->program);
    glBindVertexArray(app->vao);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glUniform1i(app->programUniformTexture, 0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, app->tex.handle);

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);

    glBindVertexArray(0);
    glUseProgram(0);
#endif


    //
    // Render pass: Draw mesh
    //

    glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 1, -1, "Draw mesh");

    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glViewport(0, 0, app->displaySize.x, app->displaySize.y);

    glUseProgram(app->meshProgram);

    for (u32 i = 0; i < app->mesh.submeshes.size(); ++i)
    {
        GLuint vao = FindVAO(app->mesh, i, app->meshProgram);
        glBindVertexArray(vao);

        const Submesh& submesh = app->mesh.submeshes[i];
        glDrawElements(GL_TRIANGLES, submesh.indices.size(), GL_UNSIGNED_INT, (void*)(u64)submesh.indexOffset);
    }

    glBindVertexArray(0);

    glUseProgram(0);

    glPopDebugGroup();

    //
    // Read pixels
    //

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
}
