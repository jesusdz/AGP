#include "entity.h"
#include "globals.h"

Entity::Entity() :
    name("Entity")
{
    for (int i = 0; i < MAX_COMPONENTS; ++i)
        components[i] = nullptr;
    transform = new Transform;
}

Entity::~Entity()
{
    delete transform;
    delete meshRenderer;
    delete terrainRenderer;
    delete lightSource;
    delete environment;
}

Component *Entity::addComponent(ComponentType componentType)
{
    Component *component = nullptr;

    switch (componentType)
    {
    case ComponentType::Transform:
        Q_ASSERT(transform == nullptr);
        component = transform = new Transform;
        break;
    case ComponentType::Environment:
        Q_ASSERT(environment == nullptr);
        component = environment = new Environment;
        break;
    case ComponentType::LightSource:
        Q_ASSERT(lightSource == nullptr);
        component = lightSource = new LightSource;
        break;;
    case ComponentType::MeshRenderer:
        Q_ASSERT(meshRenderer == nullptr);
        component = meshRenderer = new MeshRenderer;
        break;
    case ComponentType::TerrainRenderer:
        Q_ASSERT(terrainRenderer == nullptr);
        component = terrainRenderer = new TerrainRenderer;
        break;
    default:
        Q_ASSERT(false && "Invalid code path");
    }

    component->entity = this;
    return component;
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
    else if (component == environment)
    {
        delete environment;
        environment = nullptr;
    }
}

Component *Entity::findComponent(ComponentType ctype)
{
    for (int i = 0; i < MAX_COMPONENTS; ++i)
    {
        if (components[i] != nullptr && components[i]->componentType() == ctype)
        {
            return components[i];
        }
    }
    return nullptr;
}

Entity *Entity::clone() const
{
    Entity *entity = scene->addEntity(); // Global scene
    entity->name = name;
    entity->active = active;
    if (transform != nullptr) {
        //entity->addComponent(ComponentType::Transform); // transforms are created by default
        *entity->transform = *transform;
        entity->transform->entity = entity;
    }
    if (meshRenderer != nullptr) {
        entity->addComponent(ComponentType::MeshRenderer);
        *entity->meshRenderer = *meshRenderer;
        entity->meshRenderer->entity = entity;
    }
    if (lightSource != nullptr) {
        entity->addComponent(ComponentType::LightSource);
        *entity->lightSource = *lightSource;
        entity->lightSource->entity = entity;
    }
    if (environment != nullptr) {
        entity->addComponent(ComponentType::Environment);
        *entity->environment = *environment;
        entity->environment->entity = entity;
    }
    return entity;
}

void Entity::read(const QJsonObject &json)
{
    name = json["name"].toString("Entity");
    active = json["active"].toBool(true);

    if (json.contains("transform"))
    {
        //addComponent(ComponentType::Transform); // transforms are created by default
        transform->read(json["transform"].toObject());

    }
    if (json.contains("meshRenderer"))
    {
        addComponent(ComponentType::MeshRenderer);
        meshRenderer->read(json["meshRenderer"].toObject());
    }
    if (json.contains("lightSource"))
    {
        addComponent(ComponentType::LightSource);
        lightSource->read(json["lightSource"].toObject());
    }
    if (json.contains("environment"))
    {
        addComponent(ComponentType::Environment);
        environment->read(json["environment"].toObject());
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
    if (environment != nullptr)
    {
        QJsonObject jsonComponent;
        environment->write(jsonComponent);
        json["environment"] = jsonComponent;
    }
}
