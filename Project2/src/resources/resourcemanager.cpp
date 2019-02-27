#include "resourcemanager.h"
#include "mesh.h"
#include <QVector3D>


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
        QVector3D(-0.5, 1.0, 0.5), QVector3D(0,-1,0),
        QVector3D( 0.5, 1.0,-0.5), QVector3D(0,-1,0),
        QVector3D( 0.5, 1.0, 0.5), QVector3D(0,-1,0),
        QVector3D(-0.5, 1.0, 0.5), QVector3D(0,-1,0),
        QVector3D(-0.5, 1.0,-0.5), QVector3D(0,-1,0),
        QVector3D( 0.5, 1.0,-0.5), QVector3D(0,-1,0)
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

    int stride = 2 * sizeof(QVector3D);

    VertexFormat vertexFormat;
    vertexFormat.setVertexAttribute(0, 0, 3, stride);
    vertexFormat.setVertexAttribute(1, sizeof(QVector3D), 3, stride);

    Mesh *mesh = createMesh();
    mesh->name = "Triangles";
    mesh->addSubMesh(vertexFormat, tris, sizeof(tris));

    mesh = createMesh();
    mesh->name = "Cube";
    mesh->addSubMesh(vertexFormat, cube, sizeof(cube));

    mesh = createMesh();
    mesh->name = "Plane";
    mesh->addSubMesh(vertexFormat, plane, sizeof(plane));

    mesh = createMesh();
    mesh->name = "Sphere";
    mesh->addSubMesh(vertexFormat, sphere, sizeof(sphere), &sphereIndices[0][0][0], sizeof(sphereIndices));
}

ResourceManager::~ResourceManager()
{
    for (auto mesh : meshes) {
        delete mesh;
    }
}

Mesh *ResourceManager::createMesh()
{
    Mesh *mesh = new Mesh;
    meshes.push_back(mesh);
    return mesh;
}
