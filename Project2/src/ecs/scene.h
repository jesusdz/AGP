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
class Environment;
class Mesh;
class Texture;
class TextureCube;
class Material;


enum class ComponentType {
    Transform,
    MeshRenderer,
    LightSource,
    TerrainRenderer,
    Environment
};


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

    Component *findComponent(ComponentType ctype);

    void clear();

    void handleResourcesAboutToDie();

    void read(const QJsonObject &json);
    void write(QJsonObject &json);

    QVector<Entity*> entities;

    // TODO: Maybe not the best place for this stuff...
    QColor backgroundColor;
    bool renderBloom = true;
    bool renderSSAO = true;
    bool renderWater = true;
    bool renderGrid = true;
    bool renderLightSources = true;
    bool renderSelectionOutline = true;
};


// Entity //////////////////////////////////////////////////////////////

#define MAX_COMPONENTS 8

class Entity
{
public:

    Entity();
    ~Entity();

    Component *addComponent(ComponentType ctype);
    Component *findComponent(ComponentType ctype);
    void removeComponent(Component *component);

    Entity *clone() const;

    void read(const QJsonObject &json);
    void write(QJsonObject &json);

    QString name;

    union
    {
        struct
        {
            Transform *transform;
            MeshRenderer *meshRenderer;
            TerrainRenderer *terrainRenderer;
            LightSource *lightSource;
            Environment *environment;
        };
        Component *components[MAX_COMPONENTS];
    };

    bool active = true;
};


// Components //////////////////////////////////////////////////////////

class Component
{
public:
    Component() { }
    virtual ~Component() { }

    virtual ComponentType componentType() const = 0;

    virtual void read(const QJsonObject &json) = 0;
    virtual void write(QJsonObject &json) = 0;

    Entity *entity = nullptr;
};

class Transform : public Component
{
public:
    Transform();

    QMatrix4x4 matrix() const;

    ComponentType componentType() const override { return ComponentType::Transform; }

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

    ComponentType componentType() const override { return ComponentType::MeshRenderer; }

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

    ComponentType componentType() const override { return ComponentType::TerrainRenderer; }

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

    ComponentType componentType() const override { return ComponentType::LightSource; }

    void read(const QJsonObject &json) override;
    void write(QJsonObject &json) override;

    Type type = Type::Point;
    QColor color;
    float intensity = 1.0f;
    float range = 10.0f;
};

class Environment : public Component
{
public:

    Environment();

    ~Environment();

    void handleResourcesAboutToDie();

    ComponentType componentType() const override { return ComponentType::Environment; }

    void read(const QJsonObject &json) override;
    void write(QJsonObject &json) override;

    Texture *texture = nullptr;
    bool needsProcessing = false;
    TextureCube *environmentMap = nullptr;
    TextureCube *irradianceMap = nullptr;

    static Environment *instance; // NOTE(jesus): Only allowing one instance currently
};

#endif // SCENE_H
