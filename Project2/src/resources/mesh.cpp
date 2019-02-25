#include "mesh.h"
#include "opengl/functions.h"
#include <QVector3D>

SubMesh::SubMesh(VertexFormat, void *data, int bytes)
{
    vbo.create();
    vbo.bind();
    vbo.setUsagePattern(QOpenGLBuffer::UsagePattern::StaticDraw);
    vbo.allocate(data, bytes);

    // VAO: Captures state of VBOs
    vao.create();
    vao.bind();
    const GLint compCount = 3;
    const int strideBytes = 2 * 3 * sizeof(float);
    const int offsetBytes0 = 0;
    const int offsetBytes1 = 3 * sizeof(float);

    glfuncs->glEnableVertexAttribArray(0);
    glfuncs->glEnableVertexAttribArray(1);
    glfuncs->glVertexAttribPointer(0, compCount, GL_FLOAT, GL_FALSE, strideBytes, (void*)(offsetBytes0));
    glfuncs->glVertexAttribPointer(1, compCount, GL_FLOAT, GL_FALSE, strideBytes, (void*)(offsetBytes1));

    // Release
    vao.release();
    vbo.release();
}

void SubMesh::draw()
{
    vao.bind();
    glDrawArrays(GL_TRIANGLES, 0, 6);
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
}
