#include "scene.h"
#include "globals.h"


// Scene //////////////////////////////////////////////////////////////////

Scene::Scene()
{
}

Scene::~Scene()
{
    for (auto entity : entities)
    {
        delete entity;
    }
}

int Scene::numEntities() const
{
    return entities.size();
}

Entity *Scene::addEntity()
{
    Entity *entity = new Entity;
    entities.push_back(entity);
    return entity;
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
            delete entities[i];
            entities.removeAt(i);
            return;
        }
    }
}

void Scene::removeEntityAt(int index)
{
    delete entities[index];
    entities.removeAt(index);
}


// Entity /////////////////////////////////////////////////////////////////

Entity::Entity() :
    name("Entity")
{
    transform = new Transform;
    meshRenderer = nullptr;
}

Entity::~Entity()
{
    delete transform;
    delete meshRenderer;
}

void Entity::addTransformComponent()
{
    transform = new Transform;
}

void Entity::addMeshRendererComponent()
{
    meshRenderer = new MeshRenderer;
}

void Entity::removeComponent(Component *component)
{
    if (transform == component)
    {
        delete transform;
        transform = nullptr;
    }
    else if (component == meshRenderer)
    {
        delete meshRenderer;
        meshRenderer = nullptr;
    }
}

Transform::Transform() :
    position(0.0f, 0.0f, 0.0f),
    rotation(),
    scale(1.0f, 1.0f, 1.0f)
{

}

QMatrix4x4 Transform::matrix() const
{
    QMatrix4x4 mat;
    mat.translate(position);
    mat.rotate(rotation);
    mat.scale(scale);
    return mat;
}

MeshRenderer::MeshRenderer()
{
    // TODO: Remove (initialization only for testing purposes)
    if (resourceManager->meshes.empty())
        mesh = nullptr;
    else
        mesh = resourceManager->meshes[0];
}
