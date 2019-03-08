#include "material.h"
#include <QJsonObject>


const char *Material::TypeName = "Material";


Material::Material() :
    albedo(QColor::fromRgb(255, 255, 255)),
    emissive(QColor::fromRgb(0, 0, 0)),
    smoothness(0.0f)
{ }

Material::~Material()
{ }

void Material::read(const QJsonObject &json)
{
    albedo.setNamedColor( json["albedo"].toString() );
    emissive.setNamedColor( json["emissive"].toString() );
    smoothness = json["smoothness"].toDouble();
}

void Material::write(QJsonObject &json)
{
    json["albedo"] = albedo.name();
    json["emissive"] = emissive.name();
    json["smoothness"] = smoothness;
}
