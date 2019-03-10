#include "resourcemanager.h"
#include "mesh.h"
#include "material.h"
#include "texture.h"
#include "shaderprogram.h"
#include <QVector3D>
#include <cmath>
#include <QJsonArray>
#include <QJsonObject>


ResourceManager::ResourceManager()
{
    QVector3D tris[] = {
        // Triangle 1
        QVector3D(-0.5f, -0.5f, 0.0f), QVector3D(1.0f, 0.0f, 0.0f), // Vertex 1
        QVector3D( 0.5f, -0.5f, 0.0f), QVector3D(0.0f, 1.0f, 0.0f), // Vertex 2
        QVector3D( 0.0f,  0.5f, 0.0f), QVector3D(0.0f, 0.0f, 1.0f), // Vertex 3
        // Triangle 2
        QVector3D(-0.7f, -0.3f, 0.5f), QVector3D(0.5f, 0.0f, 0.0f), // Vertex 4
        QVector3D( 0.3f, -0.3f, 0.5f), QVector3D(0.0f, 0.5f, 0.0f), // Vertex 5
        QVector3D(-0.2f,  0.7f, 0.5f), QVector3D(0.0f, 0.0f, 0.5f)  // Vertex 6
    };

    QVector3D cube[] = {
        // +Z
        QVector3D(-0.5, 0.0, 0.5), QVector3D(0,0,1),
        QVector3D( 0.5, 0.0, 0.5), QVector3D(0,0,1),
        QVector3D( 0.5, 1.0, 0.5), QVector3D(0,0,1),
        QVector3D(-0.5, 0.0, 0.5), QVector3D(0,0,1),
        QVector3D( 0.5, 1.0, 0.5), QVector3D(0,0,1),
        QVector3D(-0.5, 1.0, 0.5), QVector3D(0,0,1),
        // -Z
        QVector3D(-0.5, 0.0,-0.5), QVector3D(0,0,-1),
        QVector3D( 0.5, 1.0,-0.5), QVector3D(0,0,-1),
        QVector3D( 0.5, 0.0,-0.5), QVector3D(0,0,-1),
        QVector3D(-0.5, 0.0,-0.5), QVector3D(0,0,-1),
        QVector3D(-0.5, 1.0,-0.5), QVector3D(0,0,-1),
        QVector3D( 0.5, 1.0,-0.5), QVector3D(0,0,-1),
        // +X
        QVector3D( 0.5, 0.0,-0.5), QVector3D( 1,0,0),
        QVector3D( 0.5, 1.0, 0.5), QVector3D( 1,0,0),
        QVector3D( 0.5, 0.0, 0.5), QVector3D( 1,0,0),
        QVector3D( 0.5, 0.0,-0.5), QVector3D( 1,0,0),
        QVector3D( 0.5, 1.0,-0.5), QVector3D( 1,0,0),
        QVector3D( 0.5, 1.0, 0.5), QVector3D( 1,0,0),
        // -X
        QVector3D(-0.5, 0.0,-0.5), QVector3D(-1,0,0),
        QVector3D(-0.5, 0.0, 0.5), QVector3D(-1,0,0),
        QVector3D(-0.5, 1.0, 0.5), QVector3D(-1,0,0),
        QVector3D(-0.5, 0.0,-0.5), QVector3D(-1,0,0),
        QVector3D(-0.5, 1.0, 0.5), QVector3D(-1,0,0),
        QVector3D(-0.5, 1.0,-0.5), QVector3D(-1,0,0),
        // +Y
        QVector3D(-0.5, 1.0, 0.5), QVector3D(0,1,0),
        QVector3D( 0.5, 1.0, 0.5), QVector3D(0,1,0),
        QVector3D( 0.5, 1.0,-0.5), QVector3D(0,1,0),
        QVector3D(-0.5, 1.0, 0.5), QVector3D(0,1,0),
        QVector3D( 0.5, 1.0,-0.5), QVector3D(0,1,0),
        QVector3D(-0.5, 1.0,-0.5), QVector3D(0,1,0),
        // +Y
        QVector3D(-0.5, 0.0, 0.5), QVector3D(0,-1,0),
        QVector3D( 0.5, 0.0,-0.5), QVector3D(0,-1,0),
        QVector3D( 0.5, 0.0, 0.5), QVector3D(0,-1,0),
        QVector3D(-0.5, 0.0, 0.5), QVector3D(0,-1,0),
        QVector3D(-0.5, 0.0,-0.5), QVector3D(0,-1,0),
        QVector3D( 0.5, 0.0,-0.5), QVector3D(0,-1,0)
    };

    QVector3D plane[] = {
        QVector3D(-10.0, 0.0, 10.0), QVector3D(0,1,0),
        QVector3D( 10.0, 0.0, 10.0), QVector3D(0,1,0),
        QVector3D( 10.0, 0.0,-10.0), QVector3D(0,1,0),
        QVector3D(-10.0, 0.0, 10.0), QVector3D(0,1,0),
        QVector3D( 10.0, 0.0,-10.0), QVector3D(0,1,0),
        QVector3D(-10.0, 0.0,-10.0), QVector3D(0,1,0)
    };

#define H 32
#define V 16

    static const float pi = 3.1416f;
    struct Vertex { QVector3D pos; QVector3D norm; };

    Vertex sphere[H][V + 1];
    for (int h = 0; h < H; ++h) {
        for (int v = 0; v < V + 1; ++v) {
            float nh = float(h) / H;
            float nv = float(v) / V - 0.5f;
            float angleh = 2 * pi * nh;
            float anglev = - pi * nv;
            sphere[h][v].pos.setX(sinf(angleh) * cosf(anglev));
            sphere[h][v].pos.setY(-sinf(anglev));
            sphere[h][v].pos.setZ(cosf(angleh) * cosf(anglev));
            sphere[h][v].norm = sphere[h][v].pos;
        }
    }

    unsigned int sphereIndices[H][V][6];
    for (unsigned int h = 0; h < H; ++h) {
        for (unsigned int v = 0; v < V; ++v) {
            sphereIndices[h][v][0] =  (h+0)    * (V+1) + v;
            sphereIndices[h][v][1] = ((h+1)%H) * (V+1) + v;
            sphereIndices[h][v][2] = ((h+1)%H) * (V+1) + v+1;
            sphereIndices[h][v][3] =  (h+0)    * (V+1) + v;
            sphereIndices[h][v][4] = ((h+1)%H) * (V+1) + v+1;
            sphereIndices[h][v][5] =  (h+0)    * (V+1) + v+1;
        }
    }

    VertexFormat vertexFormat;
    vertexFormat.setVertexAttribute(0, 0, 3);
    vertexFormat.setVertexAttribute(1, sizeof(QVector3D), 3);

    Mesh *mesh = createMesh();
    mesh->name = "Triangles";
    mesh->includeForSerialization = false;
    mesh->addSubMesh(vertexFormat, tris, sizeof(tris));

    mesh = createMesh();
    mesh->name = "Cube";
    mesh->includeForSerialization = false;
    mesh->addSubMesh(vertexFormat, cube, sizeof(cube));
    this->cube = mesh;

    mesh = createMesh();
    mesh->name = "Plane";
    mesh->includeForSerialization = false;
    mesh->addSubMesh(vertexFormat, plane, sizeof(plane));
    this->plane = mesh;

    mesh = createMesh();
    mesh->name = "Sphere";
    mesh->includeForSerialization = false;
    mesh->addSubMesh(vertexFormat, sphere, sizeof(sphere), &sphereIndices[0][0][0], H*V*6);
    this->sphere = mesh;


    // Pre made textures

    QImage whitePixel(1, 1, QImage::Format::Format_RGB888);
    whitePixel.setPixelColor(0, 0, QColor::fromRgb(255, 255, 255));

    QImage blackPixel(1, 1, QImage::Format::Format_RGB888);
    blackPixel.setPixelColor(0, 0, QColor::fromRgb(0, 0, 0));

    QImage normalPixel(1, 1, QImage::Format::Format_RGB888);
    normalPixel.setPixelColor(0, 0, QColor::fromRgb(128, 128, 255));

    texWhite = createTexture();
    texWhite->name = "White texture";
    texWhite->includeForSerialization = false;
    texWhite->setImage(whitePixel);

    texBlack = createTexture();
    texBlack->name = "Black texture";
    texBlack->includeForSerialization = false;
    texBlack->setImage(blackPixel);

    texNormal = createTexture();
    texNormal->name = "Normal texture";
    texNormal->includeForSerialization = false;
    texNormal->setImage(normalPixel);


    // Pre made materials

    materialWhite = createMaterial();
    materialWhite->name = "White material";
    materialWhite->includeForSerialization = false;

    materialLight = createMaterial();
    materialLight->name = "Material light";
    materialLight->emissive = QColor(255, 255, 255);
    materialLight->includeForSerialization = false;


    // Shaders

    forwardShading = createShaderProgram();
    forwardShading->name = "Forward shading";
    //forwardShading->vertexShaderFilename = ":/shaders/forward_shading.vert";
    //forwardShading->fragmentShaderFilename = ":/shaders/forward_shading.frag";
    forwardShading->vertexShaderFilename = "res/shaders/forward_shading.vert";
    forwardShading->fragmentShaderFilename = "res/shaders/forward_shading.frag";
    forwardShading->includeForSerialization = false;
}

ResourceManager::~ResourceManager()
{
    qDebug("ResourceManager deletion");
    for (auto res : resources) {
        delete res;
    }
}

Mesh *ResourceManager::createMesh()
{
    Mesh *m = new Mesh;
    resources.push_back(m);
    return m;
}

Mesh *ResourceManager::getMesh(const QString &name)
{
    for (auto res : resources)
    {
        if (res->name == name)
        {
            return res->asMesh();
        }
    }
    return nullptr;
}


Material *ResourceManager::createMaterial()
{
    Material *m = new Material;
    resources.push_back(m);
    return m;
}

Material *ResourceManager::getMaterial(const QString &name)
{
    for (auto res : resources)
    {
        if (res->name == name)
        {
            return res->asMaterial();
        }
    }
    return nullptr;
}

Texture *ResourceManager::createTexture()
{
    Texture *t = new Texture;
    resources.push_back(t);
    return t;
}

Texture *ResourceManager::loadTexture(const QString &filePath)
{
    Texture *tex = nullptr;
    for (auto res : resources)
    {
        tex = res->asTexture();
        if (tex != nullptr && tex->getFilePath() == filePath)
        {
            return tex;
        }
    }
    tex = createTexture();
    tex->loadTexture(filePath.toLatin1());
    return tex;
}

Texture *ResourceManager::getTexture(const QString &name)
{
    for (auto res : resources)
    {
        if (res->name == name)
        {
            return res->asTexture();
        }
    }
    return nullptr;
}

ShaderProgram *ResourceManager::createShaderProgram()
{
    ShaderProgram *res = new ShaderProgram;
    resources.push_back(res);
    return res;
}

ShaderProgram *ResourceManager::getShaderProgram(const QString &name)
{
    for (auto res : resources)
    {
        if (res->name == name)
        {
            return res->asShaderProgram();
        }
    }
    return nullptr;
}

void ResourceManager::reloadShaderPrograms()
{
    for (auto res : resources)
    {
        if (res->asShaderProgram())
        {
            res->asShaderProgram()->reload();
        }
    }
}

Resource *ResourceManager::createResource(const QString &type)
{
    if (type == QString::fromLatin1(Mesh::TypeName))
    {
        return createMesh();
    }
    if (type == QString::fromLatin1(Texture::TypeName))
    {
        return createTexture();
    }
    if (type == QString::fromLatin1(Material::TypeName))
    {
        return createMaterial();
    }

    qDebug("Could not create the resource of type %s", type.toStdString().c_str());
    return nullptr;
}

Resource *ResourceManager::getResource(const QString &name)
{
    for (auto res : resources)
    {
        if (res->name == name)
        {
            return res;
        }
    }
    return nullptr;
}

int ResourceManager::numResources() const
{
    return resources.size();
}

Resource *ResourceManager::resourceAt(int index)
{
    return resources[index];
}

void ResourceManager::read(const QJsonObject &json)
{
    QJsonArray listOfResources = json["resources"].toArray();
    for (auto jsonResourceValue : listOfResources)
    {
        QJsonObject jsonResource = jsonResourceValue.toObject();
        QString resourceTypeName = jsonResource["typeName"].toString();
        QString resourceName = jsonResource["name"].toString();
        Resource *res = createResource(resourceTypeName.toStdString().c_str());
        res->name = resourceName;
        res->read(jsonResource);
    }
}

void ResourceManager::write(QJsonObject &json)
{
    QJsonArray listOfResources;
    for (auto resource : resources)
    {
        if (resource->includeForSerialization)
        {
            QJsonObject jsonResource;
            jsonResource["typeName"] = QString::fromLatin1(resource->typeName());
            jsonResource["name"] = resource->name;
            resource->write(jsonResource);
            listOfResources.push_back(jsonResource);
        }
    }
    json["resources"] = listOfResources;
}

void ResourceManager::removeResourceAt(int index)
{
    resources[index]->needsRemove = true;
    resourcesToDestroy.push_back(resources[index]);
    resources.remove(index);
}

void ResourceManager::updateResources()
{
    for (auto resource : resources)
    {
        if (resource->needsUpdate)
        {
            resource->update();
            resource->needsUpdate = false;
        }
    }

    for (auto resource : resourcesToDestroy)
    {
        resource->destroy();
        delete resource;
    }
    resourcesToDestroy.clear();
}

void ResourceManager::destroyResources()
{
    for (auto resource : resources)
    {
        resource->destroy();
    }
}
