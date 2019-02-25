#ifndef OPENGLWIDGET_H
#define OPENGLWIDGET_H

#include <QOpenGLWidget>
#include <QOpenGLFunctions_3_3_Core>
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLShaderProgram>
#include <QOpenGLDebugMessage>
#include <QTimer>


class OpenGLWidget :
        public QOpenGLWidget,
        protected QOpenGLFunctions_3_3_Core
{
    Q_OBJECT
public:
    explicit OpenGLWidget(QWidget *parent = nullptr);
    ~OpenGLWidget() override;

    // Virtual methods
    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;

    // Public methods
    QString getOpenGLInfo();
    QImage getScreenshot();

signals:

public slots:

    // Not virtual
    void finalizeGL();

    void handleLoggedMessage(const QOpenGLDebugMessage &debugMessage);

private:

    void initializeRender();
    void render();
    void finalizeRender();

    QOpenGLShaderProgram program;

    QTimer timer;

};

#endif // OPENGLWIDGET_H
