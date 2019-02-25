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

    QString name;
};

#endif // RESOURCE_H
