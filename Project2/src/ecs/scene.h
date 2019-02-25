#ifndef SCENE_H
#define SCENE_H

#include <QString>
#include <QVector>

class Entity;
class Component;
class Transform;
class MeshRenderer;
class Mesh;


// Scene ///////////////////////////////////////////////////////////////

const int MAX_ENTITIES = 100;

class Scene
{
public:
    Scene();
    ~Scene();

    int numEntities() const;
    Entity *addEntity();
    Entity *entityAt(int index);
    void removeEntity(Entity * entity);
    void removeEntityAt(int index);

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

    QString name;
    Transform *transform;
    MeshRenderer *meshRenderer;
};


// Components //////////////////////////////////////////////////////////

class Component
{
public:
};

class Transform : public Component
{
public:
    Transform();

    float tx, ty, tz;
    float rx, ry, rz;
    float sx, sy, sz;
};

class MeshRenderer : public Component
{
public:

    MeshRenderer();

    Mesh *mesh = nullptr;
};

#endif // SCENE_H
