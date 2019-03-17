#include "ui/openglwidget.h"
#include <QVector3D>
#include <QVector4D>
#include <QOpenGLDebugLogger>
#include <QOpenGLShaderProgram>
#include <iostream>
#include <QFile>
#include <QMouseEvent>
#include <QKeyEvent>
#include "opengl/functions.h"
#include "resources/resourcemanager.h"
#include "resources/mesh.h"
#include "resources/material.h"
#include "resources/shaderprogram.h"
#include "resources/texture.h"
#include "ecs/scene.h"
#include "globals.h"
#include <cmath>

QOpenGLFunctions_3_3_Core *glfuncs = nullptr;

OpenGLWidget::OpenGLWidget(QWidget *parent)
    : QOpenGLWidget(parent)
{
    setMinimumSize(QSize(256, 256));
    glfuncs = this;

    for (int i = 0; i < 10; ++i) {
        mouseButtons[i] = MouseButtonState::Up;
    }
    for (int i = 0; i < 300; ++i) {
        keys[i] = KeyState::Up;
    }

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

    // Camera position
    cpos = QVector3D(0.0, 2.0, 6.0);
}

OpenGLWidget::~OpenGLWidget()
{
    glfuncs = nullptr;

    //makeCurrent();
    //finalizeGL(); // This makes the app crash...
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
}

void OpenGLWidget::resizeGL(int w, int h)
{
    // TODO: resize textures
}

void OpenGLWidget::paintGL()
{
    glClearDepth(1.0);
    glClearColor(0.8f, 0.9f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    resourceManager->updateResources();

    render();
}

void OpenGLWidget::finalizeGL()
{
    resourceManager->destroyResources();
}

void OpenGLWidget::keyPressEvent(QKeyEvent *event)
{
    if (event->key() < 300) {
        if (keys[event->key()] == KeyState::Up) {
            keys[event->key()] = KeyState::Pressed;
        }
    }
}

void OpenGLWidget::keyReleaseEvent(QKeyEvent *event)
{
    if (event->key() < 300) {
        keys[event->key()] = KeyState::Up;
    }
}

void OpenGLWidget::mousePressEvent(QMouseEvent *event)
{
    if (mouseButtons[event->button()] == MouseButtonState::Up) {
        mousex = mousex_prev = event->x();
        mousey = mousey_prev = event->y();
        mouseButtons[event->button()] = MouseButtonState::Pressed;
    }

    setFocus();
}

void OpenGLWidget::mouseMoveEvent(QMouseEvent *event)
{
    mousex = event->x();
    mousey = event->y();
}

void OpenGLWidget::mouseReleaseEvent(QMouseEvent *event)
{
    mouseButtons[event->button()] = MouseButtonState::Up;
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

static float radians(float degrees)
{
    return degrees * 3.1416f / 180.0f;
}

static bool enabledZtest = true;
static bool enabledFaceCulling = true;

void OpenGLWidget::preUpdate()
{
    bool cameraChanged = false;
    int mousex_delta = mousex - mousex_prev;
    int mousey_delta = mousey - mousey_prev;
    mousex_prev = mousex;
    mousey_prev = mousey;

    // Camera rotation
    if (mouseButtons[Qt::RightButton] == MouseButtonState::Down)
    {
        if (mousex_delta != 0 || mousey_delta != 0)
        {
            cyaw -= 0.3f * mousex_delta;
            cpitch -= 0.3f * mousey_delta;
            while (cyaw < 0.0f) cyaw += 360.0f;
            while (cyaw > 360.0f) cyaw -= 360.0f;
            if (cpitch > 89.0f) cpitch = 89.0f;
            if (cpitch < -89.0f) cpitch = -89.0f;
            cameraChanged = true;
        }
    }

    QVector3D displacementVector;

    if (keys[Qt::Key_A] == KeyState::Down) // Left
    {
        displacementVector -= QVector3D(cosf(radians(cyaw)), 0.0f, -sinf(radians(cyaw)));
        cameraChanged = true;
    }
    if (keys[Qt::Key_D] == KeyState::Down) // Right
    {
        displacementVector += QVector3D(cosf(radians(cyaw)), 0.0f, -sinf(radians(cyaw)));
        cameraChanged = true;
    }
    if (keys[Qt::Key_W] == KeyState::Down) // Front
    {
        displacementVector += QVector3D(-sinf(radians(cyaw)) * cosf(radians(cpitch)), sinf(radians(cpitch)), -cosf(radians(cyaw)) * cosf(radians(cpitch)));
        cameraChanged = true;
    }
    if (keys[Qt::Key_S] == KeyState::Down) // Back
    {
        displacementVector -= QVector3D(-sinf(radians(cyaw)) * cosf(radians(cpitch)), sinf(radians(cpitch)), -cosf(radians(cyaw)) * cosf(radians(cpitch)));
        cameraChanged = true;
    }

    // Increase speed
    displacementVector *= 5.0f;

    if (keys[Qt::Key_Z] == KeyState::Pressed)
    {
        enabledZtest = !enabledZtest;
        std::cout << "ztest " << enabledZtest << std::endl;
        cameraChanged = true;
    }

    if (keys[Qt::Key_C] == KeyState::Pressed)
    {
        enabledFaceCulling = !enabledFaceCulling;
        std::cout << "face culling " << enabledFaceCulling << std::endl;
        cameraChanged = true;
    }

    cpos += displacementVector / 60.0f;

    for (int i = 0; i < 10; ++i) {
        if (mouseButtons[i] == MouseButtonState::Pressed) {
            mouseButtons[i] = MouseButtonState::Down;
        }
    }
    for (int i = 0; i < 300; ++i) {
        if (keys[i] == KeyState::Pressed) {
            keys[i] = KeyState::Down;
        }
    }

    if (cameraChanged) {
        update();
    }
}

static void sendLightsToProgram(QOpenGLShaderProgram &program, const QMatrix4x4 &viewMatrix)
{
    QVector<int> lightType;
    QVector<QVector3D> lightPosition;
    QVector<QVector3D> lightDirection;
    QVector<QVector3D> lightColor;
    for (auto entity : scene->entities)
    {
        if (entity->lightSource != nullptr)
        {
            auto light = entity->lightSource;
            lightType.push_back(int(light->type));
            lightPosition.push_back(QVector3D(viewMatrix * entity->transform->matrix() * QVector4D(0.0, 0.0, 0.0, 1.0)));
            lightDirection.push_back(QVector3D(viewMatrix * entity->transform->matrix() * QVector4D(0.0, 1.0, 0.0, 0.0)));
            QVector3D color(light->color.redF(), light->color.greenF(), light->color.blueF());
            lightColor.push_back(color * light->intensity);
        }
    }
    if (lightPosition.size() > 0)
    {
        program.setUniformValueArray("lightType", &lightType[0], lightType.size());
        program.setUniformValueArray("lightPosition", &lightPosition[0], lightPosition.size());
        program.setUniformValueArray("lightDirection", &lightDirection[0], lightDirection.size());
        program.setUniformValueArray("lightColor", &lightColor[0], lightColor.size());
    }
    program.setUniformValue("lightCount", lightPosition.size());
}

void OpenGLWidget::render()
{
//    // Back face culling
//    if (enabledFaceCulling) {
//        glEnable(GL_CULL_FACE);
//        glCullFace(GL_BACK);
//    } else {
//        glDisable(GL_CULL_FACE);
//    }

//    // Depth test
//    if (enabledZtest) {
//        glEnable(GL_DEPTH_TEST);
//    } else {
//        glDisable(GL_DEPTH_TEST);
//    }

    // Backface culling and z-test
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glEnable(GL_DEPTH_TEST);

    QOpenGLShaderProgram &program = resourceManager->forwardShading->program;

    if (program.bind())
    {
        QMatrix4x4 cameraWorldMatrix;
        cameraWorldMatrix.translate(cpos);
        cameraWorldMatrix.rotate(cyaw, QVector3D(0.0, 1.0, 0.0));
        cameraWorldMatrix.rotate(cpitch, QVector3D(1.0, 0.0, 0.0));

        QMatrix4x4 viewMatrix = cameraWorldMatrix.inverted();
        //viewMatrix.lookAt(QVector3D(3.0, 2.0, 5.0), QVector3D(0.0, 0.0, 0.0), QVector3D(0.0, 1.0, 0.0));
        program.setUniformValue("viewMatrix", viewMatrix);

        QMatrix4x4 projectionMatrix;
        projectionMatrix.perspective(60.0f, float(width()) / height(), 0.01, 1000.0);
        program.setUniformValue("projectionMatrix", projectionMatrix);

        sendLightsToProgram(program, viewMatrix);

        for (auto entity : scene->entities)
        {
            auto meshRenderer = entity->meshRenderer;

            if (meshRenderer != nullptr)
            {
                auto mesh = meshRenderer->mesh;

                if (mesh != nullptr)
                {
                    QMatrix4x4 worldMatrix = entity->transform->matrix();
                    QMatrix4x4 worldViewMatrix = viewMatrix * worldMatrix;
                    QMatrix3x3 normalMatrix = worldViewMatrix.normalMatrix();

                    program.setUniformValue("worldMatrix", worldMatrix);
                    program.setUniformValue("worldViewMatrix", worldViewMatrix);
                    program.setUniformValue("normalMatrix", normalMatrix);

                    int materialIndex = 0;
                    for (auto submesh : mesh->submeshes)
                    {
                        // Get material from the component
                        Material *material = nullptr;
                        if (materialIndex < meshRenderer->materials.size()) {
                            material = meshRenderer->materials[materialIndex];
                        }
                        if (material == nullptr) {
                            material = resourceManager->materialWhite;
                        }
                        materialIndex++;

#define SEND_TEXTURE(uniformName, tex1, tex2, texUnit) \
    program.setUniformValue(uniformName, texUnit); \
    if (tex1 != nullptr) { \
        tex1->bind(texUnit); \
    } else { \
        tex2->bind(texUnit); \
    }

                        // Send the material to the shader
                        program.setUniformValue("albedo", material->albedo);
                        program.setUniformValue("emissive", material->emissive);
                        program.setUniformValue("specular", material->specular);
                        program.setUniformValue("smoothness", material->smoothness);
                        program.setUniformValue("bumpiness", material->bumpiness);
                        SEND_TEXTURE("albedoTexture", material->albedoTexture, resourceManager->texWhite, 0);
                        SEND_TEXTURE("emissiveTexture", material->emissiveTexture, resourceManager->texBlack, 1);
                        SEND_TEXTURE("specularTexture", material->specularTexture, resourceManager->texBlack, 2);
                        SEND_TEXTURE("normalTexture", material->normalsTexture, resourceManager->texNormal, 3);
                        SEND_TEXTURE("bumpTexture", material->bumpTexture, resourceManager->texWhite, 4);

                        submesh->draw();
                    }
                }
            }
        }

        // Render lights
        for (auto entity : scene->entities)
        {
            auto lightSource = entity->lightSource;

            if (lightSource != nullptr)
            {
                QMatrix4x4 worldMatrix = entity->transform->matrix();
                QMatrix4x4 scaleMatrix; scaleMatrix.scale(0.1, 0.1, 0.1);
                QMatrix4x4 worldViewMatrix = viewMatrix * worldMatrix * scaleMatrix;
                QMatrix3x3 normalMatrix = worldViewMatrix.normalMatrix();
                program.setUniformValue("worldMatrix", worldMatrix);
                program.setUniformValue("worldViewMatrix", worldViewMatrix);
                program.setUniformValue("normalMatrix", normalMatrix);

                for (auto submesh : resourceManager->sphere->submeshes)
                {
                    // Send the material to the shader
                    Material *material = resourceManager->materialLight;
                    program.setUniformValue("albedo", material->albedo);
                    program.setUniformValue("emissive", material->emissive);
                    program.setUniformValue("smoothness", material->smoothness);

                    submesh->draw();
                }
            }
        }

        program.release();
    }


    {
    QOpenGLShaderProgram &program = resourceManager->forwardShadingTerrain->program;

    if (program.bind())
    {
        // Render terrains
        for (auto entity : scene->entities)
        {
            QMatrix4x4 cameraWorldMatrix;
            cameraWorldMatrix.translate(cpos);
            cameraWorldMatrix.rotate(cyaw, QVector3D(0.0, 1.0, 0.0));
            cameraWorldMatrix.rotate(cpitch, QVector3D(1.0, 0.0, 0.0));

            QMatrix4x4 viewMatrix = cameraWorldMatrix.inverted();
            //viewMatrix.lookAt(QVector3D(3.0, 2.0, 5.0), QVector3D(0.0, 0.0, 0.0), QVector3D(0.0, 1.0, 0.0));
            program.setUniformValue("viewMatrix", viewMatrix);

            QMatrix4x4 projectionMatrix;
            projectionMatrix.perspective(60.0f, float(width()) / height(), 0.01, 1000.0);
            program.setUniformValue("projectionMatrix", projectionMatrix);

            sendLightsToProgram(program, viewMatrix);

            auto terrainRenderer = entity->terrainRenderer;

            if (terrainRenderer != nullptr)
            {
                auto mesh = terrainRenderer->mesh;

                if (mesh != nullptr)
                {
                    QMatrix4x4 worldMatrix = entity->transform->matrix();
                    QMatrix4x4 worldViewMatrix = viewMatrix * worldMatrix;
                    QMatrix3x3 normalMatrix = worldViewMatrix.normalMatrix();

                    program.setUniformValue("worldMatrix", worldMatrix);
                    program.setUniformValue("worldViewMatrix", worldViewMatrix);
                    program.setUniformValue("normalMatrix", normalMatrix);

                    program.setUniformValue("terrainSize", float(terrainRenderer->size));
                    program.setUniformValue("terrainResolution", terrainRenderer->texture->size());
                    program.setUniformValue("terrainMaxHeight", terrainRenderer->height);
                    program.setUniformValue("terrainHeightMap", 5);
                    terrainRenderer->texture->bind(5);

                    for (auto submesh : mesh->submeshes)
                    {
                        // Get material from the component
                        Material *material = resourceManager->materialWhite;

                        #define SEND_TEXTURE(uniformName, tex1, tex2, texUnit) \
                        program.setUniformValue(uniformName, texUnit); \
                        if (tex1 != nullptr) { \
                            tex1->bind(texUnit); \
                        } else { \
                            tex2->bind(texUnit); \
                        }

                        // Send the material to the shader
                        program.setUniformValue("albedo", material->albedo);
                        program.setUniformValue("emissive", material->emissive);
                        program.setUniformValue("specular", material->specular);
                        program.setUniformValue("smoothness", material->smoothness);
                        program.setUniformValue("bumpiness", material->bumpiness);
                        SEND_TEXTURE("albedoTexture", material->albedoTexture, resourceManager->texWhite, 0);
                        SEND_TEXTURE("emissiveTexture", material->emissiveTexture, resourceManager->texBlack, 1);
                        SEND_TEXTURE("specularTexture", material->specularTexture, resourceManager->texBlack, 2);
                        SEND_TEXTURE("normalTexture", material->normalsTexture, resourceManager->texNormal, 3);
                        SEND_TEXTURE("bumpTexture", material->bumpTexture, resourceManager->texWhite, 4);

                        submesh->draw();
                    }
                }
            }
        }

        program.release();
    }
    }
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
