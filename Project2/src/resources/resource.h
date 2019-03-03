#ifndef RESOURCE_H
#define RESOURCE_H

#include <QString>

class Mesh;
class Material;
class Texture;

class Resource
{
public:
    Resource() : name("Resource") { }
    Resource(QString n) : name(n) { }
    virtual ~Resource() { }

    virtual Mesh * asMesh() { return nullptr; }
    virtual Material * asMaterial() { return nullptr; }
    virtual Texture * asTexture() { return nullptr; }

    virtual void update() { needsUpdate = false; }
    virtual void destroy() { }

    QString name;
    bool needsUpdate = false;
};

#endif // RESOURCE_H
