#include "ui/openglwidget.h"
#include <QVector3D>
#include <QOpenGLDebugLogger>
#include <iostream>
#include <QFile>
#include <QMouseEvent>
#include <QKeyEvent>
#include "opengl/functions.h"
#include "resources/resourcemanager.h"
#include "resources/mesh.h"
#include "ecs/scene.h"
#include "globals.h"

QOpenGLFunctions_3_3_Core *glfuncs = nullptr;

OpenGLWidget::OpenGLWidget(QWidget *parent)
    : QOpenGLWidget(parent)
{
    setMinimumSize(QSize(256, 256));
    glfuncs = this;

    resourceManager = new ResourceManager();

    // Configure the timer
    connect(&timer, SIGNAL(timeout()), this, SLOT(preUpdate()));
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

    for (int i = 0; i < 300; ++i) {
        keys[i] = KeyState::Up;
    }

    // Camera position
    cpos = QVector3D(0.0, 2.0, 6.0);
}

OpenGLWidget::~OpenGLWidget()
{
    delete resourceManager;
    glfuncs = nullptr;
    makeCurrent();
    finalizeGL();
}

void OpenGLWidget::initializeGL()
{
    initializeOpenGLFunctions();

    if (context()->hasExtension(QByteArrayLiteral("GL_KHR_debug")))
    {
        QOpenGLDebugLogger *logger = new QOpenGLDebugLogger(this);
        logger->initialize(); // initializes in the current context, i.e. ctx

        connect(logger, SIGNAL(messageLogged(const QOpenGLDebugMessage &)),
                this, SLOT(handleLoggedMessage(const QOpenGLDebugMessage &)));
        logger->startLogging();
    }

    // Handle context destructions
    connect(context(), SIGNAL(aboutToBeDestroyed()),
            this, SLOT(finalizeGL()));

    initializeRender();
}

void OpenGLWidget::resizeGL(int w, int h)
{
    // TODO: resize textures
}

void OpenGLWidget::paintGL()
{
    glClearColor(0.9f, 0.85f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    updateResources();
    render();
}

void OpenGLWidget::finalizeGL()
{
    finalizeRender();
}

void OpenGLWidget::keyPressEvent(QKeyEvent *event)
{
    if (keys[event->key()] == KeyState::Up) {
        keys[event->key()] = KeyState::Pressed;
    }
}

void OpenGLWidget::keyReleaseEvent(QKeyEvent *event)
{
    keys[event->key()] = KeyState::Up;
}

static int prevx = 0;
static int prevy = 0;

void OpenGLWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::RightButton)
    {
        prevx = event->x();
        prevy = event->y();
        grabKeyboard();
    }
}

void OpenGLWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (event->buttons() & Qt::RightButton)
    {
        cyaw -= event->x() - prevx;
        cpitch -= event->y() - prevy;
        while (cyaw < 0.0f) cyaw += 360.0f;
        while (cyaw > 360.0f) cyaw -= 360.0f;
        if (cpitch > 89.0f) cpitch = 89.0f;
        if (cpitch < -89.0f) cpitch = -89.0f;
        prevx = event->x();
        prevy = event->y();
        update();
    }
}

void OpenGLWidget::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::RightButton)
    {
        releaseKeyboard();
        for (int i = 0; i < 300; ++i) {
            keys[i] = KeyState::Up;
        }
    }
}

void OpenGLWidget::handleLoggedMessage(const QOpenGLDebugMessage &debugMessage)
{
    std::cout << debugMessage.severity() << ": "
              << debugMessage.message().toStdString() << std::endl;
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

const char * const readFile(const char *filename, GLint *len)
{
    QFile file(QString::fromLatin1(filename));
    if (file.open(QFile::ReadOnly))
    {
        static char contents[1024*64];
        QByteArray array = file.readAll();
        *len = array.size();
        memcpy(contents, array.data(), *len);
        contents[*len] = '\0';
        return (const char * const)contents;
    }
    return nullptr;
}

void OpenGLWidget::initializeRender()
{
    // Program
    program.create();
    program.addShaderFromSourceFile(QOpenGLShader::Vertex, ":/shaders/forward_shading.vert");
    program.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/forward_shading.frag");
    program.link();
}

static float radians(float degrees)
{
    return degrees * 3.1416f / 180.0f;
}

void OpenGLWidget::preUpdate()
{
    QVector3D displacementVector;

    if (keys[Qt::Key_A] == KeyState::Down) // Left
    {
        displacementVector -= QVector3D(cosf(radians(cyaw)), 0.0f, -sinf(radians(cyaw)));
        update();
    }
    if (keys[Qt::Key_D] == KeyState::Down) // Right
    {
        displacementVector += QVector3D(cosf(radians(cyaw)), 0.0f, -sinf(radians(cyaw)));
        update();
    }
    if (keys[Qt::Key_W] == KeyState::Down) // Front
    {
        displacementVector += QVector3D(-sinf(radians(cyaw)) * cosf(radians(cpitch)), sinf(radians(cpitch)), -cosf(radians(cyaw)) * cosf(radians(cpitch)));
        update();
    }
    if (keys[Qt::Key_S] == KeyState::Down) // Back
    {
        displacementVector -= QVector3D(-sinf(radians(cyaw)) * cosf(radians(cpitch)), sinf(radians(cpitch)), -cosf(radians(cyaw)) * cosf(radians(cpitch)));
        update();
    }

    cpos += displacementVector / 60.0f;

    for (int i = 0; i < 300; ++i) {
        if (keys[i] == KeyState::Pressed) {
            keys[i] = KeyState::Down;
        }
    }
}

void OpenGLWidget::updateResources()
{
    for (auto resource : resourceManager->meshes)
    {
        if (resource->needsUpdate)
        {
            resource->update();
        }
    }
}

void OpenGLWidget::render()
{
    // Back face culling
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    // Depth test
    glEnable(GL_DEPTH_TEST);

    if (program.bind())
    {
        QMatrix4x4 cameraWorldMatrix;
        //cameraWorldMatrix.translate(QVector3D(3.0, 2.0, 5.0));
        cameraWorldMatrix.translate(cpos);
        cameraWorldMatrix.rotate(cyaw, QVector3D(0.0, 1.0, 0.0));
        cameraWorldMatrix.rotate(cpitch, QVector3D(1.0, 0.0, 0.0));

        QMatrix4x4 viewMatrix = cameraWorldMatrix.inverted();
        //viewMatrix.lookAt(QVector3D(3.0, 2.0, 5.0), QVector3D(0.0, 0.0, 0.0), QVector3D(0.0, 1.0, 0.0));
        program.setUniformValue("worldViewMatrix", viewMatrix);

        QMatrix4x4 projectionMatrix;
        projectionMatrix.perspective(60.0f, float(width()) / height(), 0.01, 1000.0);
        program.setUniformValue("projectionMatrix", projectionMatrix);

        for (auto entity : scene->entities)
        {
            auto meshRenderer = entity->meshRenderer;

            if (meshRenderer != nullptr)
            {
                auto mesh = meshRenderer->mesh;

                if (mesh != nullptr)
                {
                    for (auto submesh : mesh->submeshes)
                    {
                        // TODO
                        // Get material from the component

                        submesh->draw();
                    }
                }
            }
        }

        program.release();
    }
}

void OpenGLWidget::finalizeRender()
{
    // No need to release objects handled by Qt classes
}

//class OpenGLErrorGuard
//{
//    public:

//    OpenGLErrorGuard(const char *message) : msg(message) {
//        checkGLError("BEGIN", msg);
//    }

//    ~OpenGLErrorGuard() {
//        checkGLError("END", msg);
//    }

//    static void checkGLError(const char *around, const char *message);

//    const char *msg;
//};

//void OpenGLWidget::blur()
//{
//    OpenGLErrorGuard guard("blur()");

//    // Blurring OpenGL calls
//}
