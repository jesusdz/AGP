#include "ui/openglwidget_texture.h"
#include <QVector3D>

OpenGLWidgetTexture::OpenGLWidgetTexture(QWidget *parent)
    : QOpenGLWidget(parent)
{
    setMinimumSize(QSize(128, 128));
}

OpenGLWidgetTexture::~OpenGLWidgetTexture()
{
}

void OpenGLWidgetTexture::initializeGL()
{
    initializeOpenGLFunctions();

    // Program
    program.create();
    program.addShaderFromSourceFile(QOpenGLShader::Vertex, ":/shaders/forward_shading.vert");
    program.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/forward_shading.frag");
    program.link();

    // VAO: Vertex format description and state of VBOs
    vao.create();
    vao.bind();

    // VBO: Buffer with vertex data
    QVector3D data[6] = {
        QVector3D(-1.0f,-1.0f, 0.0f),
        QVector3D( 1.0f,-1.0f, 0.0f),
        QVector3D( 1.0f, 1.0f, 0.0f),
        QVector3D(-1.0f,-1.0f, 0.0f),
        QVector3D( 1.0f, 1.0f, 0.0f),
        QVector3D(-1.0f, 1.0f, 0.0f)
    };
    vbo.create();
    vbo.bind();
    vbo.setUsagePattern(QOpenGLBuffer::UsagePattern::StaticDraw);
    vbo.allocate(data, sizeof(data));

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(QVector3D), nullptr);

    // Release
    vao.release();
    vbo.release();
}

void OpenGLWidgetTexture::paintGL()
{
    glClearDepth(1.0);
    glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glDisable(GL_DEPTH_TEST);

    if (program.bind())
    {
        vao.bind();
        const int textureUnit = 0;
        // TODO: texture->bind(textureUnit);
        program.setUniformValue("texture", textureUnit);

        vao.release();
        program.release();
    }
}
