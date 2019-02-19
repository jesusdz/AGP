#include "scene.h"

Scene *g_Scene = nullptr;


// Scene //////////////////////////////////////////////////////////////////

Scene::Scene()
{
    g_Scene = this;

    for (int i = 0; i < MAX_ENTITIES; ++i)
    {
        entities[i] = nullptr;
    }
}

Scene::~Scene()
{
    g_Scene = nullptr;

    for (int i = 0; i < MAX_ENTITIES; ++i)
    {
        delete entities[i];
    }
}

int Scene::numEntities() const
{
    return size;
}

Entity *Scene::addEntity()
{
    for (int i = 0; i < MAX_ENTITIES; ++i)
    {
        if (entities[i] == nullptr)
        {
            size++;
            entities[i] = new Entity();
            return entities[i];
        }
    }
    return nullptr;
}

Entity *Scene::entityAt(int index)
{
    return entities[index];
}

void Scene::removeEntity(Entity *entity)
{
    for (int i = 0; i < MAX_ENTITIES; ++i)
    {
        if (entities[i] == entity)
        {
            size--;
            delete entities[i];
            entities[i] = nullptr;
        }
    }
    fillGaps();
}

void Scene::removeEntityAt(int index)
{
    size--;
    delete entities[index];
    entities[index] = nullptr;
    fillGaps();
}

void Scene::fillGaps()
{
    int currentEmptyIndex = 0;
    for (int i = 0; i < MAX_ENTITIES; ++i)
    {
        if (entities[i] != nullptr && i > currentEmptyIndex)
        {
            entities[currentEmptyIndex++] = entities[i];
            entities[i] = nullptr;
        }
    }
}


// Entity /////////////////////////////////////////////////////////////////

Entity::Entity() :
    name("Entity")
{
    transform = new Transform;
    shapeRenderer = nullptr;
    backgroundRenderer = nullptr;
}

Entity::~Entity()
{
    delete transform;
    delete shapeRenderer;
    delete backgroundRenderer;
}

void Entity::addTransformComponent()
{
    transform = new Transform;
}

void Entity::addShapeRendererComponent()
{
    shapeRenderer = new ShapeRenderer;
}

void Entity::addBackgroundRendererComponent()
{
    backgroundRenderer = new BackgroundRenderer;
}

void Entity::removeComponent(Component *component)
{
    if (transform == component)
    {
        delete transform;
        transform = nullptr;
    }
    else if (component == shapeRenderer)
    {
        delete shapeRenderer;
        shapeRenderer = nullptr;
    }
    else if (component == backgroundRenderer)
    {
        delete backgroundRenderer;
        backgroundRenderer = nullptr;
    }
}

Transform::Transform() :
    tx(0.0f),
    ty(0.0f),
    sx(1.0f),
    sy(1.0f)
{

}

ShapeRenderer::ShapeRenderer() :
    shape(Shape::Circle),
    size(50.0f),
    fillColor(255, 255, 255),
    strokeColor(0, 0, 0),
    strokeThickness(1.0f)
{

}

BackgroundRenderer::BackgroundRenderer() :
    color(160,220,250)
{

}
