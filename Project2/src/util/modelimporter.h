#ifndef MODELIMPORTER_H
#define MODELIMPORTER_H

#include <QString>

class Entity;
class Mesh;
class aiMesh;
class aiNode;
class aiScene;

class ModelImporter
{
public:

    ModelImporter();
    ~ModelImporter();

    Entity *import(const QString &path);

private:

    // Assimp stuff
    void processNode(aiNode *node, const aiScene *scene, Mesh *myMesh);
    void processMesh(aiMesh *mesh, const aiScene *scene, Mesh *myMesh);
};

#endif // MODELIMPORTER_H
