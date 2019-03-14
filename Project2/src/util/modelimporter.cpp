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

#if 0
    QByteArray data = file.readAll();

    const aiScene *scene = import.ReadFileFromMemory(
                data.data(), data.size(),
                aiProcess_Triangulate |
                aiProcess_FlipUVs |
                aiProcess_GenSmoothNormals |
                aiProcess_OptimizeMeshes |
                aiProcess_PreTransformVertices |
                aiProcess_ImproveCacheLocality |
                aiProcess_CalcTangentSpace,
                fileInfo.suffix().toLatin1());
#else
    const aiScene *scene = import.ReadFile(
                path.toStdString(),
                aiProcess_Triangulate |
                aiProcess_FlipUVs |
                aiProcess_GenSmoothNormals |
                aiProcess_OptimizeMeshes |
                aiProcess_PreTransformVertices |
                aiProcess_ImproveCacheLocality |
                aiProcess_CalcTangentSpace);
#endif

    // Other flags
    // - aiProcess_JoinIdenticalVertices
    // - aiProcess_SortByPType
    // - aiProcess_RemoveRedundantMaterials
    // - https://www.ics.com/blog/qt-and-opengl-loading-3d-model-open-asset-import-library-assimp

    if(!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
    {
        std::cout << "ERROR::ASSIMP::" << import.GetErrorString() << std::endl;
        return nullptr;
    }

    // Used to find material files
    directory = fileInfo.path();

    // Create a list of materials
    QVector<Material*> myMaterials(scene->mNumMaterials, nullptr);
    QVector<Material*> mySubmeshMaterials(1024, nullptr);
    for (int i = 0; i < scene->mNumMaterials; ++i)
    {
        myMaterials[i] = resourceManager->createMaterial();
        processMaterial(scene->mMaterials[i], myMaterials[i]);
    }

    // Create the mesh and process the submeshes read by Assimp
    Mesh *myMesh = resourceManager->createMesh();
    myMesh->name = fileInfo.baseName();
    processNode(scene->mRootNode, scene, myMesh, &myMaterials[0], &mySubmeshMaterials[0]);

    // Create an entity showing the mesh
    Entity *entity = ::scene->addEntity();
    entity->name = fileInfo.baseName();
    entity->addMeshRendererComponent();
    entity->meshRenderer->mesh = myMesh;
    for (int i = 0; i < myMesh->submeshes.size(); ++i)
    {
        entity->meshRenderer->materials.push_back(mySubmeshMaterials[i]);
    }

    return entity;
}

void ModelImporter::processMaterial(aiMaterial *material, Material *myMaterial)
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

    myMaterial->name = QString::fromLatin1(name.C_Str());
    myMaterial->albedo = QColor::fromRgbF(diffuseColor.r, diffuseColor.g, diffuseColor.b);
    myMaterial->emissive = QColor::fromRgbF(emissiveColor.r, emissiveColor.g, emissiveColor.b);
    myMaterial->smoothness = shininess / 256.0f;

    aiString filename;
    if (material->GetTextureCount(aiTextureType_DIFFUSE) > 0)
    {
        material->GetTexture(aiTextureType_DIFFUSE, 0, &filename);
        QString filepath = QString::fromLatin1("%0/%1").arg(directory.toLatin1().data()).arg(filename.C_Str());
        myMaterial->albedoTexture = resourceManager->loadTexture(filepath);
    }
    if (material->GetTextureCount(aiTextureType_EMISSIVE) > 0)
    {
        material->GetTexture(aiTextureType_EMISSIVE, 0, &filename);
        QString filepath = QString::fromLatin1("%0/%1").arg(directory.toLatin1().data()).arg(filename.C_Str());
        myMaterial->emissiveTexture = resourceManager->loadTexture(filepath);
    }
    if (material->GetTextureCount(aiTextureType_SPECULAR) > 0)
    {
        material->GetTexture(aiTextureType_SPECULAR, 0, &filename);
        QString filepath = QString::fromLatin1("%0/%1").arg(directory.toLatin1().data()).arg(filename.C_Str());
        myMaterial->specularTexture = resourceManager->loadTexture(filepath);
    }
    if (material->GetTextureCount(aiTextureType_NORMALS) > 0)
    {
        material->GetTexture(aiTextureType_NORMALS, 0, &filename);
        QString filepath = QString::fromLatin1("%0/%1").arg(directory.toLatin1().data()).arg(filename.C_Str());
        myMaterial->normalsTexture = resourceManager->loadTexture(filepath);
    }
    if (material->GetTextureCount(aiTextureType_HEIGHT) > 0)
    {
        material->GetTexture(aiTextureType_HEIGHT, 0, &filename);
        QString filepath = QString::fromLatin1("%0/%1").arg(directory.toLatin1().data()).arg(filename.C_Str());
        myMaterial->bumpTexture = resourceManager->loadTexture(filepath);
    }
    if (myMaterial->normalsTexture == nullptr && myMaterial->bumpTexture != nullptr)
    {
        // Create normal map from the height texture
        QImage bumpMap = myMaterial->bumpTexture->getImage();
        QImage normalMap(bumpMap.size(), QImage::Format_RGB888);
        const int w = normalMap.width();
        const int h = normalMap.height();
        const float bumpiness = 2.0f;
        for (int y = 0; y < h; ++y) {
            for (int x = 0; x < w; ++x) {

                // surrounding indices
                const int il = (x + w - 1) % w;
                const int ir = (x + 1) % w;
                const int ib = (y + 1) % h;
                const int it = (y + w - 1) % h;

                // surrounding pixels
                float tl = qRed( bumpMap.pixel(il, it) ) / 255.0f; // top left
                float  l = qRed( bumpMap.pixel(il,  y) ) / 255.0f; // left
                float bl = qRed( bumpMap.pixel(il, ib) ) / 255.0f; // bottom left
                float  t = qRed( bumpMap.pixel(x,  it) ) / 255.0f; // top
                float  b = qRed( bumpMap.pixel(x,  ib) ) / 255.0f; // bottom
                float tr = qRed( bumpMap.pixel(ir, it) ) / 255.0f; // top right
                float  r = qRed( bumpMap.pixel(ir,  y) ) / 255.0f; // right
                float br = qRed( bumpMap.pixel(ir, ib) ) / 255.0f; // bottom right

                // sobel filter
                const float dX = (tl + 2.0 * l + bl) - (tr + 2.0 * r + br);
                const float dY = (bl + 2.0 * b + br) - (tl + 2.0 * t + tr);
                const float dZ = 1.0/bumpiness;

                QVector3D n(dX, dY, dZ);
                n.normalize();
                n = n* 0.5 + QVector3D(0.5f, 0.5f, 0.5f);

                normalMap.setPixelColor(x, y, QColor::fromRgbF(n.x(), n.y(), n.z()));
            }
        }
        myMaterial->normalsTexture = resourceManager->createTexture();
        myMaterial->normalsTexture->setImage(normalMap);
    }
}

void ModelImporter::processNode(aiNode *node, const aiScene *scene, Mesh *myMesh, Material **myMaterials, Material **mySubmeshMaterials)
{
    // process all the node's meshes (if any)
    for(unsigned int i = 0; i < node->mNumMeshes; i++)
    {
        aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
        processMesh(mesh, scene, myMesh, myMaterials, mySubmeshMaterials);
    }

    // then do the same for each of its children
    for(unsigned int i = 0; i < node->mNumChildren; i++)
    {
        processNode(node->mChildren[i], scene, myMesh, myMaterials, mySubmeshMaterials);
    }
}

void ModelImporter::processMesh(aiMesh *mesh, const aiScene *scene, Mesh *myMesh, Material **myMaterials, Material **mySubmeshMaterials)
{
    QVector<float> vertices;
    QVector<unsigned int> indices;

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
            vertices.push_back(mesh->mBitangents[i].x);
            vertices.push_back(mesh->mBitangents[i].y);
            vertices.push_back(mesh->mBitangents[i].z);
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
    if(mesh->mMaterialIndex >= 0)
    {
        mySubmeshMaterials[myMesh->submeshes.size()] = myMaterials[mesh->mMaterialIndex];
    }

    // create the vertex format
    VertexFormat vertexFormat;
    vertexFormat.setVertexAttribute(0, 0, 3);
    vertexFormat.setVertexAttribute(1, 3 * sizeof(float), 3);
    if (hasTexCoords)
    {
        vertexFormat.setVertexAttribute(2, 6 * sizeof(float), 2);
    }
    if (hasTangentSpace)
    {
        vertexFormat.setVertexAttribute(3, 8 * sizeof(float), 3);
        vertexFormat.setVertexAttribute(4, 11 * sizeof(float), 3);
    }

    // add the submesh into the mesh
    myMesh->addSubMesh(
            vertexFormat,
            &vertices[0], vertices.size() * sizeof(float),
            &indices[0], indices.size());
}
