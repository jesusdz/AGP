#include "ui/openglwidget.h"
#include <QVector3D>
#include <QOpenGLDebugLogger>
#include <iostream>
#include <QFile>
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

//    // Configure the timer
//    connect(&timer, SIGNAL(timeout()), this, SLOT(update()));
//    if(format().swapInterval() == -1)
//    {
//        // V_blank synchronization not available (tearing likely to happen)
//        qDebug("Swap Buffers at v_blank not available: refresh at approx 60fps.");
//        timer.setInterval(17);
//    }
//    else
//    {
//        qInfo("V_blank synchronization available");
//        timer.setInterval(0);
//    }
//    timer.start();
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

    render();
}

void OpenGLWidget::finalizeGL()
{
    finalizeRender();
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
    program.addShaderFromSourceFile(QOpenGLShader::Vertex, ":/shaders/shader1_vert");
    program.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/shader1_frag");
    program.link();

    // VBO
    QVector3D vertices[] = {
        // Triangle 1
        QVector3D(-0.5f, -0.5f, 0.0f), QVector3D(1.0f, 0.0f, 0.0f), // Vertex 1
        QVector3D( 0.5f, -0.5f, 0.0f), QVector3D(0.0f, 1.0f, 0.0f), // Vertex 2
        QVector3D( 0.0f,  0.5f, 0.0f), QVector3D(0.0f, 0.0f, 1.0f), // Vertex 3
        // Triangle 2
        QVector3D(-0.7f, -0.3f, 0.5f), QVector3D(0.5f, 0.0f, 0.0f), // Vertex 4
        QVector3D( 0.3f, -0.3f, 0.5f), QVector3D(0.0f, 0.5f, 0.0f), // Vertex 5
        QVector3D(-0.2f,  0.7f, 0.5f), QVector3D(0.0f, 0.0f, 0.5f)  // Vertex 6
    };

    Mesh *mesh = resourceManager->createMesh();
    mesh->addSubMesh(VertexFormat::PositionsColors, vertices, sizeof(vertices));
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
