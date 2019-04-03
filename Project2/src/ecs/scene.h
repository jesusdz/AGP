#ifndef SCENE_H
#define SCENE_H

#include <QColor>
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
class TerrainRenderer;
class LightSource;
class Mesh;
class Texture;
class Material;


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

    void clear();

    void handleResourcesAboutToDie();

    void read(const QJsonObject &json);
    void write(QJsonObject &json);

    QVector<Entity*> entities;

    QColor backgroundColor;
};


// Entity //////////////////////////////////////////////////////////////

class Entity
{
public:

    Entity();
    ~Entity();

    void addTransformComponent();
    void addMeshRendererComponent();
    void addTerrainRendererComponent();
    void addLightSourceComponent();
    void removeComponent(Component *component);

    void read(const QJsonObject &json);
    void write(QJsonObject &json);

    QString name;
    Transform *transform = nullptr;
    MeshRenderer *meshRenderer = nullptr;
    TerrainRenderer *terrainRenderer = nullptr;
    LightSource *lightSource = nullptr;
    bool active = true;
};


// Components //////////////////////////////////////////////////////////

class Component
{
public:
    Component() { }
    virtual ~Component() { }

    virtual void read(const QJsonObject &json) = 0;
    virtual void write(QJsonObject &json) = 0;

    Entity *entity = nullptr;
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
    QVector<Material*> materials;
};

class TerrainRenderer : public Component
{
public:

    TerrainRenderer();

    void updateMesh();

    void handleResourcesAboutToDie();

    void read(const QJsonObject &json) override;
    void write(QJsonObject &json) override;

    Mesh *mesh = nullptr;
    Texture *texture = nullptr;
    float size = 100.0;
    float height = 20.0;
    int gridResolution = 500;
};

class LightSource : public Component
{
public:

    enum class Type { Point, Directional };

    LightSource();

    void read(const QJsonObject &json) override;
    void write(QJsonObject &json) override;

    Type type = Type::Point;
    QColor color;
    float intensity = 1.0f;
    float range = 10.0f;
};

#endif // SCENE_H
