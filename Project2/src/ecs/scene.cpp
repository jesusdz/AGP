#include "scene.h"
#include "globals.h"
#include "resources/mesh.h"
#include "resources/texture.h"
#include "resources/texturecube.h"
#include "resources/material.h"
#include "globals.h"
#include <QJsonArray>


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

void Scene::removeEntityAt(int index)
{
    delete entities[index];
    entities.removeAt(index);
}

Component *Scene::findComponent(ComponentType ctype)
{
   for (auto entity : entities) {
       auto comp = entity->findComponent(ctype);
       if (comp != nullptr) return comp;
   }
   return nullptr;
}

void Scene::clear()
{
    for (auto entity : entities)
    {
        delete entity;
    }
    entities.clear();
}

void Scene::handleResourcesAboutToDie()
{
    for (auto entity : entities)
    {
        if (entity->meshRenderer)
        {
            entity->meshRenderer->handleResourcesAboutToDie();
        }
        if (entity->terrainRenderer)
        {
            entity->terrainRenderer->handleResourcesAboutToDie();
        }
        if (entity->environment)
        {
            entity->environment->handleResourcesAboutToDie();
        }
    }
}

void Scene::read(const QJsonObject &json)
{
    QJsonArray listOfEntities = json["hierarchy"].toArray();
    for (auto jsonEntity : listOfEntities)
    {
        Entity *entity = addEntity();
        entity->read(jsonEntity.toObject());
    }
}

void Scene::write(QJsonObject &json)
{
    QJsonArray listOfEntities;
    for (auto entity : entities)
    {
        QJsonObject jsonEntity;
        entity->write(jsonEntity);
        listOfEntities.push_back(jsonEntity);
    }
    json["hierarchy"] = listOfEntities;
}
