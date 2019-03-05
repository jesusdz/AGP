#include "scene.h"
#include "globals.h"
#include "resources/mesh.h"
#include <QJsonArray>


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

void Scene::removeEntityAt(int index)
{
    delete entities[index];
    entities.removeAt(index);
}

void Scene::handleResourcesAboutToDie()
{
    for (auto entity : entities)
    {
        if (entity->meshRenderer)
        {
            entity->meshRenderer->handleResourcesAboutToDie();
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

void Entity::read(const QJsonObject &json)
{
    name = json["name"].toString();

    if (json.contains("transform"))
    {
        addTransformComponent();
        transform->read(json["transform"].toObject());

    }
    if (json.contains("meshRenderer"))
    {
        addMeshRendererComponent();
        meshRenderer->read(json["meshRenderer"].toObject());
    }
}

void Entity::write(QJsonObject &json)
{
    json["name"] = name;

    if (transform != nullptr)
    {
        QJsonObject jsonComponent;
        transform->write(jsonComponent);
        json["transform"] = jsonComponent;
    }
    if (meshRenderer != nullptr)
    {
        QJsonObject jsonComponent;
        meshRenderer->write(jsonComponent);
        json["meshRenderer"] = jsonComponent;
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

void Transform::read(const QJsonObject &json)
{
    QJsonArray jsonPosition = json["position"].toArray();
    QJsonArray jsonRotation = json["rotation"].toArray();
    QJsonArray jsonScale = json["scale"].toArray();
    position = QVector3D(jsonPosition[0].toDouble(), jsonPosition[1].toDouble(), jsonPosition[2].toDouble());
    rotation = QQuaternion::fromEulerAngles(jsonRotation[0].toDouble(), jsonRotation[1].toDouble(), jsonRotation[2].toDouble());
    scale = QVector3D(jsonScale[0].toDouble(), jsonScale[1].toDouble(), jsonScale[2].toDouble());
}

void Transform::write(QJsonObject &json)
{
    json["position"] = QJsonArray({position.x(), position.y(), position.z()});
    json["rotation"] = QJsonArray({rotation.toEulerAngles().x(), rotation.toEulerAngles().y(), rotation.toEulerAngles().z()});
    json["scale"] = QJsonArray({scale.x(), scale.y(), scale.z()});
}

MeshRenderer::MeshRenderer()
{
}

void MeshRenderer::handleResourcesAboutToDie()
{
    if (mesh->needsRemove) { mesh = nullptr; }
}

void MeshRenderer::read(const QJsonObject &json)
{
    QString meshName = json["mesh"].toString();
    mesh = resourceManager->getMesh(meshName);
}

void MeshRenderer::write(QJsonObject &json)
{
    json["mesh"] = mesh->name;
}
