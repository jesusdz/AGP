#ifndef RESOURCE_H
#define RESOURCE_H

#include <QString>

class Mesh;

class Resource
{
public:
    Resource() : name("Resource") { }
    Resource(QString n) : name(n) { }
    virtual ~Resource() { }

    virtual Mesh * asMesh() { return nullptr; }

    virtual void update() { needsUpdate = false; }

    QString name;
    bool needsUpdate = false;
};

#endif // RESOURCE_H
