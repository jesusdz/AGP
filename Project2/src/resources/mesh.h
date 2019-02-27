#ifndef MESH_H
#define MESH_H

#include "Resource.h"
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>
#include <QVector>

static const int MAX_VERTEX_ATTRIBUTES = 8;

struct VertexAttribute
{
    bool enabled = false;
    int offset = 0;
    int ncomp = 0;
    int stride = 0;
};

class VertexFormat
{
public:

    void setVertexAttribute(int location, int offset, int ncomp, int stride)
    {
        attribute[location].enabled = true;
        attribute[location].offset = offset;
        attribute[location].ncomp = ncomp;
        attribute[location].stride = stride;
        size += ncomp * sizeof(float);
    }

    VertexAttribute attribute[MAX_VERTEX_ATTRIBUTES];
    int size = 0;
};

class SubMesh
{
public:
    SubMesh(VertexFormat vertexFormat, void *data, int size);
    SubMesh(VertexFormat vertexFormat, void *data, int size, unsigned int *indexes, int size_indexes);
    ~SubMesh();

    void update();
    void draw();
    void destroy();

private:

    unsigned char *data = nullptr;
    size_t data_size = 0;

    unsigned int *indices = nullptr;
    size_t indices_size = 0;

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

    void update() override;
    void destroy() override;

    void addSubMesh(VertexFormat vertexFormat, void *data, int bytes);
    void addSubMesh(VertexFormat vertexFormat, void *data, int bytes, unsigned int *indexes, int bytes_indexes);

    QVector<SubMesh*> submeshes;
};

#endif // MESH_H
