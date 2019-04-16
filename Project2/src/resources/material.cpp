#include "material.h"
#include "texture.h"
#include "resourcemanager.h"
#include "globals.h"
#include <QJsonObject>


const char *Material::TypeName = "Material";


Material::Material() :
    albedo(QColor::fromRgb(255, 255, 255)),
    emissive(QColor::fromRgb(0, 0, 0)),
    specular(QColor::fromRgb(255, 255, 255)),
    smoothness(0.0f),
    metalness(0.0f),
    bumpiness(0.0f),
    tiling(1.0, 1.0)
{ }

Material::~Material()
{ }

#define TEXTURE_GUID(tex) (tex != nullptr)?tex->guid.toString():QUuid().toString()

void Material::write(QJsonObject &json)
{
    json["albedo"] = albedo.name();
    json["emissive"] = emissive.name();
    json["specular"] = specular.name();
    json["albedoTexture"]   = TEXTURE_GUID(albedoTexture);
    json["emissiveTexture"] = TEXTURE_GUID(emissiveTexture);
    json["specularTexture"] = TEXTURE_GUID(specularTexture);
    json["normalTexture"]   = TEXTURE_GUID(normalsTexture);
    json["bumpTexture"]     = TEXTURE_GUID(bumpTexture);
    json["smoothness"] = smoothness;
    json["metalness"] = metalness;
    json["bumpiness"] = bumpiness;

    QJsonObject jsonTiling;
    jsonTiling["x"] = tiling.x();
    jsonTiling["y"] = tiling.y();
    json["tiling"] = jsonTiling;
}

void Material::read(const QJsonObject &json)
{
    albedo.setNamedColor( json["albedo"].toString() );
    emissive.setNamedColor( json["emissive"].toString() );
    specular.setNamedColor( json["specular"].toString() );
    smoothness = json["smoothness"].toDouble();
    metalness = json["metalness"].toDouble();
    bumpiness = json["bumpiness"].toDouble();

    QJsonObject jsonTiling = json["tiling"].toObject();
    if (!jsonTiling.isEmpty()) {
        tiling.setX( jsonTiling["x"].toDouble(1.0) );
        tiling.setY( jsonTiling["y"].toDouble(1.0) );
    }
}

void Material::link(const QJsonObject &json)
{
    albedoTexture = resourceManager->getTexture(json["albedoTexture"].toString());
    emissiveTexture = resourceManager->getTexture(json["emissiveTexture"].toString());
    specularTexture = resourceManager->getTexture(json["specularTexture"].toString());
    normalsTexture = resourceManager->getTexture(json["normalTexture"].toString());
    bumpTexture = resourceManager->getTexture(json["bumpTexture"].toString());

    createNormalFromBump();
}

void Material::createNormalFromBump()
{
    if (normalsTexture == nullptr && bumpTexture != nullptr)
    {
        // Create normal map from the height texture
        QImage bumpMap = bumpTexture->getImage();
        QImage normalMap(bumpMap.size(), QImage::Format_RGB888);
        const int w = normalMap.width();
        const int h = normalMap.height();
        const float bumpiness = 2.0f;
        for (int y = 0; y < h; ++y) {
            for (int x = 0; x < w; ++x) {

                // surrounding indices
                const int il = (x + w - 1) % w;
                const int ir = (x + 1) % w;
                const int ib = (y + 1) % h;
                const int it = (y + w - 1) % h;

                // surrounding pixels
                float tl = qRed( bumpMap.pixel(il, it) ) / 255.0f; // top left
                float  l = qRed( bumpMap.pixel(il,  y) ) / 255.0f; // left
                float bl = qRed( bumpMap.pixel(il, ib) ) / 255.0f; // bottom left
                float  t = qRed( bumpMap.pixel(x,  it) ) / 255.0f; // top
                float  b = qRed( bumpMap.pixel(x,  ib) ) / 255.0f; // bottom
                float tr = qRed( bumpMap.pixel(ir, it) ) / 255.0f; // top right
                float  r = qRed( bumpMap.pixel(ir,  y) ) / 255.0f; // right
                float br = qRed( bumpMap.pixel(ir, ib) ) / 255.0f; // bottom right

                // sobel filter
                const float dX = (tl + 2.0 * l + bl) - (tr + 2.0 * r + br);
                const float dY = (bl + 2.0 * b + br) - (tl + 2.0 * t + tr);
                const float dZ = 1.0/bumpiness;

                QVector3D n(dX, dY, dZ);
                n.normalize();
                n = n* 0.5 + QVector3D(0.5f, 0.5f, 0.5f);

                normalMap.setPixelColor(x, y, QColor::fromRgbF(n.x(), n.y(), n.z()));
            }
        }

        // Test to see the saved file
        //normalMap.save(bumpTexture->name + QString("NORM.png"));

        normalsTexture = resourceManager->createTexture();
        normalsTexture->name = bumpTexture->name + "-NORM-auto";
        normalsTexture->setImage(normalMap);
    }
}
