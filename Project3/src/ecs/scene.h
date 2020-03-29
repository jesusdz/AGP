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
    void removeEntityAt(int index);

    Component *findComponent(ComponentType ctype);

    void clear();

    void handleResourcesAboutToDie();

    void read(const QJsonObject &json);
    void write(QJsonObject &json);

    QVector<Entity*> entities;

    // TODO: Maybe not the best place for this stuff...
    QColor backgroundColor;
    bool renderBloom = true;
    bool renderSSAO = true;
    bool renderWater = true;
    bool renderGrid = true;
    bool renderLightSources = true;
    bool renderSelectionOutline = true;
};

#endif // SCENE_H
