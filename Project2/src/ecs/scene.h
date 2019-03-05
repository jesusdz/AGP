#ifndef SCENE_H
#define SCENE_H

#include <QString>
#include <QVector>
#include <QVector3D>
#include <QMatrix4x4>
#include <QQuaternion>
#include <QJsonObject>

class Entity;
class Component;
class Transform;
class MeshRenderer;
class Mesh;


// Scene ///////////////////////////////////////////////////////////////

class Scene
{
public:
    Scene();
    ~Scene();

    int numEntities() const;
    Entity *addEntity();
    Entity *entityAt(int index);
    void removeEntityAt(int index);

    void handleResourcesAboutToDie();

    void read(const QJsonObject &json);
    void write(QJsonObject &json);

    QVector<Entity*> entities;
};


// Entity //////////////////////////////////////////////////////////////

class Entity
{
public:

    Entity();
    ~Entity();

    void addTransformComponent();
    void addMeshRendererComponent();
    void removeComponent(Component *component);

    void read(const QJsonObject &json);
    void write(QJsonObject &json);

    QString name;
    Transform *transform;
    MeshRenderer *meshRenderer;
};


// Components //////////////////////////////////////////////////////////

class Component
{
public:
    Component() { }
    virtual ~Component() { }

    virtual void read(const QJsonObject &json) = 0;
    virtual void write(QJsonObject &json) = 0;
};

class Transform : public Component
{
public:
    Transform();

    QMatrix4x4 matrix() const;

    void read(const QJsonObject &json) override;
    void write(QJsonObject &json) override;

    QVector3D position;
    QQuaternion rotation;
    QVector3D scale;
};

class MeshRenderer : public Component
{
public:

    MeshRenderer();

    void handleResourcesAboutToDie();

    void read(const QJsonObject &json) override;
    void write(QJsonObject &json) override;

    Mesh *mesh = nullptr;
};

#endif // SCENE_H
