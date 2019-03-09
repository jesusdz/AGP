#include "util/modelimporter.h"
#include "resources/resourcemanager.h"
#include "resources/mesh.h"
#include "resources/material.h"
#include "resources/texture.h"
#include "ecs/scene.h"
#include "globals.h"
#include <QFile>
#include <QFileInfo>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <iostream>


ModelImporter::ModelImporter()
{

}

ModelImporter::~ModelImporter()
{

}

Entity* ModelImporter::import(const QString &path)
{
    Assimp::Importer import;

    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        std::cout << "Could not open file for read: " << path.toStdString() << std::endl;
        return nullptr;
    }

    QFileInfo fileInfo(file);

    QByteArray data = file.readAll();

    const aiScene *scene = import.ReadFileFromMemory(
                data.data(), data.size(),
                aiProcess_Triangulate |
                aiProcess_FlipUVs |
                aiProcess_GenSmoothNormals |
                //aiProcess_RemoveRedundantMaterials |
                aiProcess_OptimizeMeshes |
                aiProcess_PreTransformVertices |
                aiProcess_ImproveCacheLocality,
                fileInfo.suffix().toLatin1());

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
        return nullptr;
    }

    // Used to find material files
    //directory = path.substr(0, path.find_last_of('/'));

    // Create the mesh and process the submeshes read by Assimp
    Mesh *myMesh = resourceManager->createMesh();
    myMesh->name = fileInfo.baseName();
    processNode(scene->mRootNode, scene, myMesh);

    // Create an entity showing the mesh
    Entity *entity = ::scene->addEntity();
    entity->name = fileInfo.baseName();
    entity->addMeshRendererComponent();
    entity->meshRenderer->mesh = myMesh;
    entity->transform->scale = QVector3D(0.01, 0.01, 0.01); // TODO: Remove this scaling

    //filePath = path;

    return entity;
}

void ModelImporter::processNode(aiNode *node, const aiScene *scene, Mesh *myMesh)
{
    // process all the node's meshes (if any)
    for(unsigned int i = 0; i < node->mNumMeshes; i++)
    {
        aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
        processMesh(mesh, scene, myMesh);
    }

    // then do the same for each of its children
    for(unsigned int i = 0; i < node->mNumChildren; i++)
    {
        processNode(node->mChildren[i], scene, myMesh);
    }
}

void ModelImporter::processMesh(aiMesh *mesh, const aiScene *scene, Mesh *myMesh)
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
    // process material
    if(mesh->mMaterialIndex >= 0)
    {
        aiMaterial *material = scene->mMaterials[mesh->mMaterialIndex];
//        vector<Texture> diffuseMaps = loadMaterialTextures(material,
//                                            aiTextureType_DIFFUSE, "texture_diffuse");
//        textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
//        vector<Texture> specularMaps = loadMaterialTextures(material,
//                                            aiTextureType_SPECULAR, "texture_specular");
//        textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
    }

    VertexFormat vertexFormat;
    vertexFormat.setVertexAttribute(0, 0, 3);
    vertexFormat.setVertexAttribute(1, 3 * sizeof(float), 3);

    if (hasTexCoords)
    {
        vertexFormat.setVertexAttribute(2, 6 * sizeof(float), 2);
    }

    myMesh->addSubMesh(
            vertexFormat,
            &vertices[0], vertices.size() * sizeof(float),
            &indices[0], indices.size());
}
