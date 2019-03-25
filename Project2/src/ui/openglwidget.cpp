#include "ui/openglwidget.h"
#include <QOpenGLDebugLogger>
#include "rendering/deferredrenderer.h"
#include "resources/resourcemanager.h"
#include "resources/texture.h"
#include "globals.h"
#include "input.h"
#include "ecs/camera.h"
#include <iostream>


QOpenGLFunctions_3_3_Core *gl = nullptr;


OpenGLWidget::OpenGLWidget(QWidget *parent)
    : QOpenGLWidget(parent)
{
    setMinimumSize(QSize(256, 256));
    gl = this;

    // Configure the timer
    connect(&timer, SIGNAL(timeout()), this, SLOT(frame()));
    if(format().swapInterval() == -1)
    {
        // V_blank synchronization not available (tearing likely to happen)
        qDebug("Swap Buffers at v_blank not available: refresh at approx 60fps.");
        timer.setInterval(17);
    }
    else
    {
        qInfo("V_blank synchronization available");
        timer.setInterval(0);
    }
    timer.start();

    input = new Input();
    camera = new Camera();
    renderer = new DeferredRenderer();

    // global
    ::input = input;
    ::camera = camera;
}

OpenGLWidget::~OpenGLWidget()
{
    delete input;
    delete camera;
    delete renderer;

    gl = nullptr;

    //makeCurrent();
    //finalizeGL(); // This makes the app crash...
}

void OpenGLWidget::initializeGL()
{
    initializeOpenGLFunctions();

    OpenGLState::initialize();

    if (context()->hasExtension(QByteArrayLiteral("GL_KHR_debug")))
    {
        QOpenGLDebugLogger *logger = new QOpenGLDebugLogger(this);
        logger->initialize(); // initializes in the current context, i.e. ctx

        connect(logger, SIGNAL(messageLogged(const QOpenGLDebugMessage &)),
                this, SLOT(handleLoggedMessage(const QOpenGLDebugMessage &)));
        logger->startLogging();
    }

    // Handle context destructions
    connect(context(), SIGNAL(aboutToBeDestroyed()), this, SLOT(finalizeGL()));

    // Backface culling and z-test
    gl->glEnable(GL_CULL_FACE);
    gl->glCullFace(GL_BACK);
    gl->glEnable(GL_DEPTH_TEST);
    gl->glDisable(GL_BLEND);

    renderer->initialize();
}

void OpenGLWidget::resizeGL(int w, int h)
{
    camera->setViewportSize(w, h);
    renderer->resize(w, h);
}

void OpenGLWidget::paintGL()
{
    resourceManager->updateResources();

    camera->prepareMatrices();

    renderer->render(camera);
}

void OpenGLWidget::finalizeGL()
{
    resourceManager->destroyResources();
}

void OpenGLWidget::keyPressEvent(QKeyEvent *event)
{
    input->keyPressEvent(event);
}

void OpenGLWidget::keyReleaseEvent(QKeyEvent *event)
{
    input->keyReleaseEvent(event);
}

void OpenGLWidget::mousePressEvent(QMouseEvent *event)
{
    input->mousePressEvent(event);
    setFocus();
}

void OpenGLWidget::mouseMoveEvent(QMouseEvent *event)
{
    input->mouseMoveEvent(event);
}

void OpenGLWidget::mouseReleaseEvent(QMouseEvent *event)
{
    input->mouseReleaseEvent(event);
}

void OpenGLWidget::enterEvent(QEvent *)
{
    grabKeyboard();
}

void OpenGLWidget::leaveEvent(QEvent *)
{
    releaseKeyboard();
}

void OpenGLWidget::handleLoggedMessage(const QOpenGLDebugMessage &debugMessage)
{
    std::cout << debugMessage.severity() << ": " << debugMessage.message().toStdString() << std::endl;
}

QString OpenGLWidget::getOpenGLInfo()
{
    makeCurrent();

    QString info;
    info += "OpenGL version:\n";
    info += (const char *)glGetString(GL_VERSION);
    info += "\n\n";

    info += "OpenGL renderer:\n";
    info += (const char *)glGetString(GL_RENDERER);
    info += "\n\n";

    info += "OpenGL vendor:\n";
    info += (const char *)glGetString(GL_VENDOR);
    info += "\n\n";

    info += "OpenGL GLSL version:\n";
    info += (const char *)glGetString(GL_SHADING_LANGUAGE_VERSION);
    info += "\n\n";

    info += "OpenGL extensions:\n";
    GLint num_extensions;
    glGetIntegerv(GL_NUM_EXTENSIONS, &num_extensions);
    for (int i = 0; i < num_extensions; ++i)
    {
        info += QString("%0\n").arg((const char *)glGetStringi(GL_EXTENSIONS, GLuint(i)));
    }
    info += "\n";

    // Information about the surface format
    info += QString("R buffer size: %0").arg(context()->format().redBufferSize());
    info += "\n";
    info += QString("G buffer size: %0").arg(context()->format().greenBufferSize());
    info += "\n";
    info += QString("B buffer size: %0").arg(context()->format().blueBufferSize());
    info += "\n";
    info += QString("A buffer size: %0").arg(context()->format().alphaBufferSize());
    info += "\n";
    info += QString("Depth buffer size: %0").arg(context()->format().depthBufferSize());
    info += "\n";
    info += QString("Stencil buffer size: %0").arg(context()->format().stencilBufferSize());
    info += "\n";

    return info;
}

QImage OpenGLWidget::getScreenshot()
{
    makeCurrent();
    return grabFramebuffer();
}

QVector<QString> OpenGLWidget::getTextureNames()
{
    QVector<QString> textureNames = renderer->getTextures();
    return textureNames;
}

void OpenGLWidget::showTextureWithName(QString textureName)
{
    renderer->showTexture(textureName);
}

void OpenGLWidget::frame()
{
    bool cameraChanged = camera->preUpdate();

    if (cameraChanged) {
        update();
    }

    input->postUpdate();
}
