#ifndef RESOURCEMANAGER_H
#define RESOURCEMANAGER_H

#include <QVector>

class Resource;
class Mesh;
class Material;
class Texture;
class QJsonObject;

class ResourceManager
{
public:
    ResourceManager();
    ~ResourceManager();

    Mesh *createMesh();
    Mesh *getMesh(const QString &name);

    Material *createMaterial();
    Material *getMaterial(const QString &name);

    Texture *createTexture();
    Texture *getTexture(const QString &name);

    Resource *createResource(const QString &type);
    Resource *getResource(const QString &name);

    int numResources() const;
    Resource *resourceAt(int index);
    void removeResourceAt(int index);

    // Perform OpenGL calls
    void updateResources();
    void destroyResources();

    // Serialization
    void read(const QJsonObject &json);
    void write(QJsonObject &json);

    QVector<Resource*> resources;

    // Pre-made meshes
    Mesh *tris = nullptr;
    Mesh *cube = nullptr;
    Mesh *plane = nullptr;
    Mesh *sphere = nullptr;

    // Pre-made textures
    Texture *texWhite = nullptr;
    Texture *texBlack = nullptr;
    Texture *texNormal = nullptr;

private:

    QVector<Resource*> resourcesToDestroy;
};

#endif // RESOURCEMANAGER_H
