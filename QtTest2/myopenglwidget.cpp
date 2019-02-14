#include "myopenglwidget.h"

//#pragma comment( lib, "OpenGL32.lib" )

MyOpenGLWidget::MyOpenGLWidget(QWidget *parent)
    : QOpenGLWidget(parent)
{

}

void MyOpenGLWidget::initializeGL()
{

}

void MyOpenGLWidget::resizeGL(int width, int height)
{
}

void MyOpenGLWidget::paintGL()
{
    glClearColor(0.5f, 0.7f, 0.9f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
}
