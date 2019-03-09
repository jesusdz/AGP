#ifndef OPENGLWIDGET_H
#define OPENGLWIDGET_H

#include <QOpenGLWidget>
#include <QOpenGLFunctions_3_3_Core>
#include <QOpenGLDebugMessage>
#include <QVector3D>
#include <QTimer>


class OpenGLWidget :
        public QOpenGLWidget,
        protected QOpenGLFunctions_3_3_Core
{
    Q_OBJECT
public:
    explicit OpenGLWidget(QWidget *parent = nullptr);
    ~OpenGLWidget() override;

    // Virtual OpenGL methods
    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;

    // Virtual event methods
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void enterEvent(QEvent *event) override;
    void leaveEvent(QEvent *event) override;

    // Public methods
    QString getOpenGLInfo();
    QImage getScreenshot();

signals:

public slots:

    // Not virtual
    void preUpdate();
    void finalizeGL();

    void handleLoggedMessage(const QOpenGLDebugMessage &debugMessage);

private:

    void render();

    QTimer timer;

    // Camera parameters
    float cyaw = 0.0f;
    float cpitch = 0.0f;
    QVector3D cpos;

    // Keyboard state
    enum class KeyState { Up, Pressed, Down };
    KeyState keys[300];

    // Mouse state
    enum class MouseButtonState { Up, Pressed, Down };
    MouseButtonState mouseButtons[10];
    int mousex = 0;
    int mousey = 0;
    int mousex_prev = 0;
    int mousey_prev = 0;
};

#endif // OPENGLWIDGET_H
