#include "openglwidget.h"
#include "scene.h"


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

}

void OpenGLWidget::resizeGL(int w, int h)
{
    update();
}

void OpenGLWidget::paintGL()
{
    glClearColor(0.9f, 0.85f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
}
