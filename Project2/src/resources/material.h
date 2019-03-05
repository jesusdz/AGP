#ifndef MATERIAL_H
#define MATERIAL_H

#include "resource.h"
#include <QColor>

class Material : public Resource
{
public:
    Material();
    ~Material() override;

    Material * asMaterial() override { return this; }

    void read(const QJsonObject &json) override;
    void write(QJsonObject &json) override;

    QColor albedo;   // RGB color
    float roughness; // from 0.0 to 1.0
};

#endif // MATERIAL_H
