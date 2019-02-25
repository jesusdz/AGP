#ifndef MESH_H
#define MESH_H

#include "Resource.h"
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>
#include <QVector>

enum class VertexFormat
{
    Positions,
    PositionsColors
};

class SubMesh
{
public:
    SubMesh(VertexFormat vertexFormat, void *data, int bytes);

    void draw();

private:

    VertexFormat vertexFormat;
    QOpenGLBuffer vbo;
    QOpenGLBuffer ibo;
    QOpenGLVertexArrayObject vao;
};

class Mesh : public Resource
{
public:
    Mesh();
    ~Mesh() override;

    Mesh * asMesh() override { return this; }

    void addSubMesh(VertexFormat vertexFormat, void *data, int bytes);

    QVector<SubMesh*> submeshes;
};

#endif // MESH_H
