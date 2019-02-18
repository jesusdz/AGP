#ifndef SCENE_H
#define SCENE_H

class Entity;
class Component;
class Transform;
class ShapeRenderer;
class BackgroundRenderer;

const int MAX_ENTITIES = 100;

class Scene
{
public:
    Scene();
    ~Scene();

    Entity *addEntity();

    Entity *entities[MAX_ENTITIES];
};

class Entity
{
public:

    void addTransformComponent();
    void addShapeRendererComponent();
    void addBackgroundRendererComponent();

    Transform *transform;
    ShapeRenderer *shapeRenderer;
    BackgroundRenderer *backgroundRenderer;
};

class Component
{
public:
};

class Transform : public Component
{
public:
    Transform(float px, float py) : x(px), y(py) { }
    float x, y;
};

class ShapeRenderer : public Component
{
public:
};

class BackgroundRenderer : public Component
{
public:
};

extern Scene *g_Scene;

#endif // SCENE_H
