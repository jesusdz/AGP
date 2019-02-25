#include "resourcemanager.h"
#include "mesh.h"


ResourceManager::ResourceManager()
{

}

ResourceManager::~ResourceManager()
{
    for (auto mesh : meshes) {
        delete mesh;
    }
}

Mesh *ResourceManager::createMesh()
{
    Mesh *mesh = new Mesh;
    meshes.push_back(mesh);
    return mesh;
}
