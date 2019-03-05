#include "mesh.h"
#include "opengl/functions.h"
#include <QVector2D>
#include <QVector3D>
#include <QFile>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <iostream>


SubMesh::SubMesh(VertexFormat vf, void *in_data, int in_data_size) :
    ibo(QOpenGLBuffer::Type::IndexBuffer)
{
    vertexFormat = vf;
    data_size = size_t(in_data_size);
    data = new unsigned char[data_size];
    memcpy(data, in_data, data_size);
}

SubMesh::SubMesh(VertexFormat vf, void *in_data, int in_data_size, unsigned int *in_indices, int in_indices_count) :
    ibo(QOpenGLBuffer::Type::IndexBuffer)
{
    vertexFormat = vf;

    data_size = size_t(in_data_size);
    data = new unsigned char[data_size];
    memcpy(data, in_data, data_size);

    indices_count = size_t(in_indices_count);
    indices = new unsigned int[indices_count];
    memcpy(indices, in_indices, indices_count * sizeof(unsigned int));
}

SubMesh::~SubMesh()
{
    delete[] data;
    delete[] indices;
}

void SubMesh::update()
{
    // VAO: Vertex format description and state of VBOs
    vao.create();
    vao.bind();

    // VBO: Buffer with vertex data
    vbo.create();
    vbo.bind();
    vbo.setUsagePattern(QOpenGLBuffer::UsagePattern::StaticDraw);
    vbo.allocate(data, int(data_size));
    delete[] data;
    data = nullptr;

    // IBO: Buffer with indexes
    if (indices != nullptr)
    {
        ibo.create();
        ibo.bind();
        ibo.setUsagePattern(QOpenGLBuffer::UsagePattern::StaticDraw);
        ibo.allocate(indices, int(indices_count * sizeof(unsigned int)));
        delete[] indices;
        indices = nullptr;
    }

    for (int location = 0; location < MAX_VERTEX_ATTRIBUTES; ++location)
    {
        VertexAttribute &attr = vertexFormat.attribute[location];

        if (attr.enabled)
        {
            glfuncs->glEnableVertexAttribArray(GLuint(location));
            glfuncs->glVertexAttribPointer(GLuint(location), attr.ncomp, GL_FLOAT, GL_FALSE, vertexFormat.size, (void *) (attr.offset));
        }
    }

    // Release
    vao.release();
    vbo.release();
    if (ibo.isCreated()) {
        ibo.release();
    }
}

void SubMesh::draw()
{
    int num_vertices = data_size / vertexFormat.size;
    vao.bind();
    if (indices_count > 0) {
        glfuncs->glDrawElements(GL_TRIANGLES, indices_count, GL_UNSIGNED_INT, nullptr);
    } else {
        glfuncs->glDrawArrays(GL_TRIANGLES, 0, num_vertices);
    }
    vao.release();
}

void SubMesh::destroy()
{
    if (vbo.isCreated()) { vbo.destroy(); }
    if (ibo.isCreated()) { ibo.destroy(); }
    if (vao.isCreated()) { vao.destroy(); }
}

Mesh::Mesh()
{

}

Mesh::~Mesh()
{
    for (auto submesh : submeshes)
    {
        delete submesh;
    }
}

void Mesh::addSubMesh(VertexFormat vertexFormat, void *data, int bytes)
{
    submeshes.push_back(new SubMesh(vertexFormat, data, bytes));
    needsUpdate = true;
}

void Mesh::addSubMesh(VertexFormat vertexFormat, void *data, int data_size, unsigned int *indices, int indices_size)
{
    submeshes.push_back(new SubMesh(vertexFormat, data, data_size, indices, indices_size));
    needsUpdate = true;
}

void Mesh::loadModel(const char *path)
{
    Assimp::Importer import;
   // const aiScene *scene = import.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);

    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        std::cout << "Could not open file for read: " << path << std::endl;
        return;
    }

    QByteArray data = file.readAll();

    const aiScene *scene = import.ReadFileFromMemory(
                data.data(), data.size(),
                aiProcess_Triangulate |
                aiProcess_FlipUVs |
                aiProcess_GenSmoothNormals |
                //aiProcess_RemoveRedundantMaterials |
                aiProcess_OptimizeMeshes |
                aiProcess_PreTransformVertices |
                aiProcess_ImproveCacheLocality ,
                ".obj");

//    // Other options
//    // https://www.ics.com/blog/qt-and-opengl-loading-3d-model-open-asset-import-library-assimp
//    const aiScene* scene = importer.ReadFile(pathToFile.toStdString(),
//            aiProcess_GenSmoothNormals |
//            aiProcess_CalcTangentSpace |
//            aiProcess_Triangulate |
//            aiProcess_JoinIdenticalVertices |
//            aiProcess_SortByPType
//            );

    if(!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
    {
        std::cout << "ERROR::ASSIMP::" << import.GetErrorString() << std::endl;
        return;
    }

    // Used to find material files
    //directory = path.substr(0, path.find_last_of('/'));

    processNode(scene->mRootNode, scene);
    needsUpdate = true;
}

void Mesh::processNode(aiNode *node, const aiScene *scene)
{
    // process all the node's meshes (if any)
    for(unsigned int i = 0; i < node->mNumMeshes; i++)
    {
        aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
        submeshes.push_back(processMesh(mesh, scene));
    }

    // then do the same for each of its children
    for(unsigned int i = 0; i < node->mNumChildren; i++)
    {
        processNode(node->mChildren[i], scene);
    }
}

SubMesh * Mesh::processMesh(aiMesh *mesh, const aiScene *scene)
{
    QVector<float> vertices;
    QVector<unsigned int> indices;

    bool hasTexCoords = false;

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

//    QVector<Texture> textures;
//    // process material
//    if(mesh->mMaterialIndex >= 0)
//    {
//        aiMaterial *material = scene->mMaterials[mesh->mMaterialIndex];
//        vector<Texture> diffuseMaps = loadMaterialTextures(material,
//                                            aiTextureType_DIFFUSE, "texture_diffuse");
//        textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
//        vector<Texture> specularMaps = loadMaterialTextures(material,
//                                            aiTextureType_SPECULAR, "texture_specular");
//        textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
//    }

    VertexFormat vertexFormat;
    vertexFormat.setVertexAttribute(0, 0, 3);
    vertexFormat.setVertexAttribute(1, 3 * sizeof(float), 3);

    if (hasTexCoords)
    {
        vertexFormat.setVertexAttribute(2, 6 * sizeof(float), 2);
    }

    return new SubMesh(vertexFormat,
            &vertices[0], vertices.size() * sizeof(float),
            &indices[0], indices.size());
}

void Mesh::update()
{
    for (auto submesh : submeshes)
    {
        submesh->update();
    }

    Resource::update();
}

void Mesh::destroy()
{
    for (auto submesh : submeshes)
    {
        submesh->destroy();
    }
}

void Mesh::read(const QJsonObject &json)
{
    // TODO
}

void Mesh::write(QJsonObject &json)
{
    // TODO
}
