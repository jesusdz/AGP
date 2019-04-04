#include "scene.h"
#include "globals.h"
#include "resources/mesh.h"
#include "resources/texture.h"
#include "resources/material.h"
#include "globals.h"
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
}

Entity::~Entity()
{
    delete transform;
    delete meshRenderer;
    delete terrainRenderer;
    delete lightSource;
}

void Entity::addTransformComponent()
{
    transform = new Transform;
    transform->entity = this;
}

void Entity::addMeshRendererComponent()
{
    meshRenderer = new MeshRenderer;
    meshRenderer->entity = this;
}

void Entity::addTerrainRendererComponent()
{
    terrainRenderer = new TerrainRenderer;
    terrainRenderer->entity = this;
}

void Entity::addLightSourceComponent()
{
    lightSource = new LightSource;
    lightSource->entity = this;
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
    else if (component == lightSource)
    {
        delete lightSource;
        lightSource = nullptr;
    }
}

Entity *Entity::clone() const
{
    Entity *entity = scene->addEntity();
    entity->name = name;
    entity->active = active;
    if (transform != nullptr) {
        entity->addTransformComponent();
        *entity->transform = *transform;
        entity->transform->entity = entity;
    }
    if (meshRenderer != nullptr) {
        entity->addMeshRendererComponent();
        *entity->meshRenderer = *meshRenderer;
        entity->meshRenderer->entity = entity;
    }
    if (lightSource != nullptr) {
        entity->addLightSourceComponent();
        *entity->lightSource = *lightSource;
        entity->lightSource->entity = entity;
    }
    return entity;
}

void Entity::read(const QJsonObject &json)
{
    name = json["name"].toString("Entity");
    active = json["active"].toBool(true);

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
    if (json.contains("lightSource"))
    {
        addLightSourceComponent();
        lightSource->read(json["lightSource"].toObject());
    }
}

void Entity::write(QJsonObject &json)
{
    json["name"] = name;
    json["active"] = active;

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
    if (lightSource != nullptr)
    {
        QJsonObject jsonComponent;
        lightSource->write(jsonComponent);
        json["lightSource"] = jsonComponent;
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
    materials.clear();
    QUuid meshGuid = json["mesh"].toString();
    QJsonArray jsonMaterialsArray = json["materials"].toArray();
    for (int i = 0; i < jsonMaterialsArray.size(); ++i)
    {
        QUuid guid = jsonMaterialsArray[i].toString();
        materials.push_back(resourceManager->getMaterial(guid));
    }
    mesh = resourceManager->getMesh(meshGuid);
}

void MeshRenderer::write(QJsonObject &json)
{
    json["mesh"] = mesh->guid.toString();
    QJsonArray jsonMaterialsArray;
    for (int i = 0; i < materials.size(); ++i)
    {
        QUuid materialGuid;
        if (materials[i] != nullptr)
        {
            materialGuid = materials[i]->guid;
        }
        jsonMaterialsArray.push_back(materialGuid.toString());
    }
    json["materials"] = jsonMaterialsArray;
}



TerrainRenderer::TerrainRenderer()
{
}

void TerrainRenderer::updateMesh()
{
    resourceManager->destroyResource(mesh);

    const int res = gridResolution;
    QVector<QVector3D> terrain(res * res * 3 * 2);
    for (int i = 0; i < res; ++i) {
        for (int j = 0; j < res; ++j) {
            terrain[(i * res + j)* 6 + 0] = QVector3D(j,   0.0, i)   * size / static_cast<float>(res);
            terrain[(i * res + j)* 6 + 1] = QVector3D(j,   0.0, i+1) * size / static_cast<float>(res);
            terrain[(i * res + j)* 6 + 2] = QVector3D(j+1, 0.0, i+1) * size / static_cast<float>(res);
            terrain[(i * res + j)* 6 + 3] = QVector3D(j,   0.0, i)   * size / static_cast<float>(res);
            terrain[(i * res + j)* 6 + 4] = QVector3D(j+1, 0.0, i+1) * size / static_cast<float>(res);
            terrain[(i * res + j)* 6 + 5] = QVector3D(j+1, 0.0, i)   * size / static_cast<float>(res);
        }
    }

    VertexFormat vertexFormat;
    vertexFormat.setVertexAttribute(0, 0, 3);

    mesh = resourceManager->createMesh();
    mesh->name = "Terrain";
    mesh->includeForSerialization = false;
    mesh->addSubMesh(vertexFormat, &terrain[0], terrain.size()*sizeof(QVector3D));
}

void TerrainRenderer::handleResourcesAboutToDie()
{
    if (mesh->needsRemove) { mesh = nullptr; }
    if (texture->needsRemove) { texture = nullptr; }
}

void TerrainRenderer::read(const QJsonObject &json)
{
    QUuid meshGuid = json["mesh"].toString();
    mesh = resourceManager->getMesh(meshGuid);
    QUuid textureGuid = json["texture"].toString();
    texture = resourceManager->getTexture(textureGuid);
}

void TerrainRenderer::write(QJsonObject &json)
{
    json["mesh"] = mesh->guid.toString();
    json["texture"] = mesh->guid.toString();
}



LightSource::LightSource() :
    color(255, 255, 255, 255)
{

}

void LightSource::read(const QJsonObject &json)
{
    type = (Type) json["type"].toInt();
    color = QColor(json["color"].toString());
    intensity = json["intensity"].toDouble(1.0);
    range = json["range"].toDouble(10.0);
}

void LightSource::write(QJsonObject &json)
{
    json["type"] = (int)type;
    json["color"] = color.name();
    json["intensity"] = intensity;
    json["range"] = range;
}
