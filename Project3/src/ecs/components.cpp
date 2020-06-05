#include "components.h"
#include "resources/mesh.h"
#include "resources/material.h"
#include "resources/texture.h"
#include "resources/texturecube.h"
#include "globals.h"
#include <QJsonArray>




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
    scene->renderListChanged = true;
}

void MeshRenderer::handleResourcesAboutToDie()
{
    if (mesh->needsRemove)
    {
        mesh = nullptr;
    }

    for (int i = 0; i < materials.size(); ++i)
    {
        if (materials[i] && materials[i]->needsRemove)
        {
            materials[i] = nullptr;
        }
    }
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
    scene->renderListChanged = true;
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

void MeshRenderer::aboutToDelete()
{
    scene->renderListChanged = true;
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
    castsShadows = json["castsShadows"].toBool(false);
}

void LightSource::write(QJsonObject &json)
{
    json["type"] = (int)type;
    json["color"] = color.name();
    json["intensity"] = intensity;
    json["range"] = range;
    json["castsShadows"] = castsShadows;
}



Environment * Environment::instance = nullptr;

Environment::Environment()
{
    if (instance == nullptr)
    {
        instance = this;
    }
}

Environment::~Environment()
{
    if (instance == this)
    {
        instance = nullptr;
    }
}

void Environment::handleResourcesAboutToDie()
{
    if (texture->needsRemove) { texture = nullptr; }
}

void Environment::read(const QJsonObject &json)
{
    QUuid textureGuid = json["texture"].toString();
    texture = resourceManager->getTexture(textureGuid);
    scene->environmentChanged = true;
}

void Environment::write(QJsonObject &json)
{
    json["texture"] = texture->guid.toString();
}

void Environment::aboutToDelete()
{
    scene->environmentChanged = true;
}
