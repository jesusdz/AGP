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
    Transform() : x(0), y (0) { }
    Transform(float px, float py) : x(px), y(py) { }
    float x, y;
};

enum class Shape { Square, Circle, Triangle };

class ShapeRenderer : public Component
{
public:

    Shape shape = Shape::Circle;
    float size = 50.0f;
};

class BackgroundRenderer : public Component
{
public:

    QColor color;
};

// Global access to scene
extern Scene *g_Scene;

#endif // SCENE_H
