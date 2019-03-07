#include "material.h"
#include <QJsonObject>


const char *Material::TypeName = "Material";


Material::Material() :
    albedo(QColor::fromRgb(255, 255, 255)),
    emissive(QColor::fromRgb(0, 0, 0))
{ }

Material::~Material()
{ }

void Material::read(const QJsonObject &json)
{
    albedo.setNamedColor( json["albedo"].toString() );
    smoothness = json["smoothness"].toDouble();
}

void Material::write(QJsonObject &json)
{
    json["albedo"] = albedo.name();
    json["smoothness"] = smoothness;
}
