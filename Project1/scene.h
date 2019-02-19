#ifndef SCENE_H
#define SCENE_H

#include <QString>
#include <QColor>

class Entity;
class Component;
class Transform;
class ShapeRenderer;
class BackgroundRenderer;


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


private:

    Entity *entities[MAX_ENTITIES];
    int size = 0;

    void fillGaps();
};


// Entity //////////////////////////////////////////////////////////////

class Entity
{
public:

    Entity();
    ~Entity();

    void addTransformComponent();
    void addShapeRendererComponent();
    void addBackgroundRendererComponent();
    void removeComponent(Component *component);

    QString name;
    Transform *transform;
    ShapeRenderer *shapeRenderer;
    BackgroundRenderer *backgroundRenderer;
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

    float tx, ty;
    float sx, sy;
};

enum class Shape { Square, Circle, Triangle };
enum class StrokeStyle { Solid, Dashed };

class ShapeRenderer : public Component
{
public:

    ShapeRenderer();

    Shape shape = Shape::Circle;
    float size = 50.0f;
    QColor fillColor;
    QColor strokeColor;
    float strokeThickness = 1.0f;
    StrokeStyle strokeStyle = StrokeStyle::Solid;
};

class BackgroundRenderer : public Component
{
public:

    BackgroundRenderer();

    QColor color;
};

// Global access to scene
extern Scene *g_Scene;

#endif // SCENE_H
