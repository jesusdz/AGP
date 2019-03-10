#ifndef MATERIAL_H
#define MATERIAL_H

#include "resource.h"
#include <QColor>

class Texture;

class Material : public Resource
{
public:

    static const char *TypeName;

    Material();
    ~Material() override;

    const char *typeName() const override { return TypeName; }

    Material * asMaterial() override { return this; }

    void read(const QJsonObject &json) override;
    void write(QJsonObject &json) override;

    QColor albedo;           // RGB color
    QColor emissive;         // Emissive color
    QColor specular;         // Specular color
    float smoothness = 0.0f; // from 0.0 to 1.0

    // Textures
    Texture *albedoTexture = nullptr;
    Texture *emissiveTexture = nullptr;
    Texture *specularTexture = nullptr;
    Texture *normalsTexture = nullptr;
    Texture *bumpTexture = nullptr;
};

#endif // MATERIAL_H
