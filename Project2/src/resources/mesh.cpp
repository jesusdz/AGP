#include "mesh.h"
#include "opengl/functions.h"
#include <QVector3D>

SubMesh::SubMesh(VertexFormat vf, void *in_data, int in_data_size) :
    ibo(QOpenGLBuffer::Type::IndexBuffer)
{
    vertexFormat = vf;
    data_size = size_t(in_data_size);
    data = new unsigned char[data_size];
    memcpy(data, in_data, data_size);
}

SubMesh::SubMesh(VertexFormat vf, void *in_data, int bytes, unsigned int *in_indices, int in_indices_size) :
    ibo(QOpenGLBuffer::Type::IndexBuffer)
{
    vertexFormat = vf;

    data_size = size_t(bytes);
    data = new unsigned char[data_size];
    memcpy(data, in_data, data_size);

    indices_size = size_t(in_indices_size);
    indices = new unsigned int[indices_size];
    memcpy(indices, in_indices, indices_size);
}

SubMesh::~SubMesh()
{
    delete[] data;
    delete[] indices;
}

void SubMesh::update()
{
    // VAO: Vertex format description and state of VBOs
    vao.create();
    vao.bind();

    // VBO: Buffer with vertex data
    vbo.create();
    vbo.bind();
    vbo.setUsagePattern(QOpenGLBuffer::UsagePattern::StaticDraw);
    vbo.allocate(data, int(data_size));
    delete[] data;
    data = nullptr;

    // IBO: Buffer with indexes
    if (indices != nullptr)
    {
        ibo.create();
        ibo.bind();
        ibo.setUsagePattern(QOpenGLBuffer::UsagePattern::StaticDraw);
        ibo.allocate(indices, int(indices_size));
        delete[] indices;
        indices = nullptr;
    }

    for (int location = 0; location < MAX_VERTEX_ATTRIBUTES; ++location)
    {
        VertexAttribute &attr = vertexFormat.attribute[location];

        if (attr.enabled)
        {
            glfuncs->glEnableVertexAttribArray(GLuint(location));
            glfuncs->glVertexAttribPointer(GLuint(location), attr.ncomp, GL_FLOAT, GL_FALSE, attr.stride, (void *) (attr.offset));
        }
    }

    // Release
    vao.release();
    vbo.release();
    if (ibo.isCreated()) {
        ibo.release();
    }
}

void SubMesh::draw()
{
    int num_vertices = data_size / vertexFormat.size;
    int num_indices = indices_size / sizeof(unsigned int);
    vao.bind();
    if (indices_size > 0) {
        glfuncs->glDrawElements(GL_TRIANGLES, num_indices, GL_UNSIGNED_INT, nullptr);
    } else {
        glfuncs->glDrawArrays(GL_TRIANGLES, 0, num_vertices);
    }
    vao.release();
}

Mesh::Mesh()
{

}

Mesh::~Mesh()
{
    for (auto submesh : submeshes)
    {
        delete submesh;
    }
}

void Mesh::addSubMesh(VertexFormat vertexFormat, void *data, int bytes)
{
    submeshes.push_back(new SubMesh(vertexFormat, data, bytes));
    needsUpdate = true;
}

void Mesh::addSubMesh(VertexFormat vertexFormat, void *data, int data_size, unsigned int *indices, int indices_size)
{
    submeshes.push_back(new SubMesh(vertexFormat, data, data_size, indices, indices_size));
    needsUpdate = true;
}

void Mesh::update()
{
    for (auto submesh : submeshes)
    {
        submesh->update();
    }

    Resource::update();
}
