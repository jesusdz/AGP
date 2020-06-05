#ifndef SCENE_H
#define SCENE_H

#include <QVector>
#include <QJsonObject>

class Entity;
class Component;

#include "entity.h"

class Scene
{
public:
    Scene();
    ~Scene();

    int numEntities() const;
    Entity *addEntity();
    Entity *entityAt(int index);
    Entity *entityWithId(int id);
    void removeEntityAt(int index);

    Component *findComponent(ComponentType ctype);

    void clear();

    void handleResourcesAboutToDie();

    void read(const QJsonObject &json);
    void write(QJsonObject &json);

    QVector<Entity*> entities;

    // TODO: Maybe not the best place for this stuff...
    bool renderBloom = true;
    QColor backgroundColor;
    float bloomRadius = 16.0f;
    float bloomLod0Intensity = 0.5f;
    float bloomLod1Intensity = 0.5f;
    float bloomLod2Intensity = 0.5f;
    float bloomLod3Intensity = 0.5f;
    float bloomLod4Intensity = 0.5f;
    bool renderSSAO = true;
    bool renderWater = true;
    bool renderGrid = true;
    bool renderLightSources = true;
    bool renderSelectionOutline = true;

    bool environmentChanged = true;
    bool renderListChanged = true;

    void clearFlags()
    {
        environmentChanged = false;
        bool renderListChanged = true;
    }
};

#endif // SCENE_H
