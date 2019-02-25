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

    QVector<Mesh*> meshes;
};

#endif // RESOURCEMANAGER_H
