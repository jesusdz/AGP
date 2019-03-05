#ifndef RESOURCE_H
#define RESOURCE_H

#include <QString>

class Mesh;
class Material;
class Texture;
class QJsonObject;

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

    virtual void read(const QJsonObject &) { }
    virtual void write(QJsonObject &) { }

    QString name;
    bool needsUpdate = false;
    bool includeForSerialization = true;
};

#endif // RESOURCE_H
