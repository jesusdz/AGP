#ifndef RESOURCEMANAGER_H
#define RESOURCEMANAGER_H

#include <QVector>

class Mesh;

class ResourceManager
{
public:
    ResourceManager();
    ~ResourceManager();

    Mesh *createMesh();

    void updateResources();
    void destroyResources();

    QVector<Mesh*> meshes;

    // Pre-made meshes
    Mesh *tris = nullptr;
    Mesh *cube = nullptr;
    Mesh *plane = nullptr;
    Mesh *sphere = nullptr;
};

#endif // RESOURCEMANAGER_H
