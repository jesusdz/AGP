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
    transform = nullptr;
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
