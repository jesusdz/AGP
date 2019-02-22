#include "openglwidget.h"
#include "scene.h"
#include <QOpenGLContext>
#include <QSurface>
#include <iostream>


OpenGLWidget::OpenGLWidget(QWidget *parent) : QOpenGLWidget(parent)
{
}

QSize OpenGLWidget::sizeHint() const
{
    return QSize(256, 256);
}

QSize OpenGLWidget::minimumSizeHint() const
{
    return QSize(64, 64);
}

void OpenGLWidget::initializeGL()
{
    std::cout << "OpenGL information:" << std::endl;
    std::cout << glGetString(GL_VERSION) << std::endl;
    const GLubyte *str = glGetString(GL_EXTENSIONS);
    if (str == nullptr)
    {
        std::cout << "Error in GL_EXTENSIONS" << std::endl;
    } else {
        std::cout << str << std::endl;
    }
    std::cout << context()->format().redBufferSize() << std::endl;
    std::cout << context()->format().greenBufferSize() << std::endl;
    std::cout << context()->format().blueBufferSize() << std::endl;
    std::cout << context()->format().alphaBufferSize() << std::endl;
    std::cout << context()->format().depthBufferSize() << std::endl;
}

void OpenGLWidget::resizeGL(int w, int h)
{
    //update();
}

void OpenGLWidget::paintGL()
{
    glClearColor(0.9f, 0.85f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
}
