#include "deferredrenderer.h"
#include "ecs/scene.h"
#include "ecs/camera.h"
#include "resources/material.h"
#include "resources/mesh.h"
#include "resources/texture.h"
#include "resources/shaderprogram.h"
#include "resources/resourcemanager.h"
#include "framebufferobject.h"
#include "gl.h"
#include "globals.h"
#include <QVector>
#include <QVector3D>
#include <QOpenGLShaderProgram>
#include <QOpenGLTexture>


DeferredRenderer::DeferredRenderer()
{
    fbo = nullptr;

    // List of textures
    addTexture("Final render"); // Light texture
    addTexture("Albedo / Occlusion");
    addTexture("Specular / Roughness");
    addTexture("Normals");
    addTexture("Depth");
    addTexture("Temp 1");
}

DeferredRenderer::~DeferredRenderer()
{
    delete fbo;
}

void DeferredRenderer::initialize()
{
    OpenGLErrorGuard guard("ForwardRenderer::initialize()");

    // Create programs

    materialProgram = resourceManager->createShaderProgram();
    materialProgram->name = "Deferred material";
    materialProgram->vertexShaderFilename = "res/shaders/deferred_material.vert";
    materialProgram->fragmentShaderFilename = "res/shaders/deferred_material.frag";
    materialProgram->includeForSerialization = false;

    lightingProgram = resourceManager->createShaderProgram();
    lightingProgram->name = "Deferred lighting";
    lightingProgram->vertexShaderFilename = "res/shaders/deferred_lighting.vert";
    lightingProgram->fragmentShaderFilename = "res/shaders/deferred_lighting.frag";
    lightingProgram->includeForSerialization = false;

    backgroundProgram = resourceManager->createShaderProgram();
    backgroundProgram->name = "Background";
    backgroundProgram->vertexShaderFilename = "res/shaders/background.vert";
    backgroundProgram->fragmentShaderFilename = "res/shaders/background.frag";
    backgroundProgram->includeForSerialization = false;

    boxProgram = resourceManager->createShaderProgram();
    boxProgram->name = "Box";
    boxProgram->vertexShaderFilename = "res/shaders/box.vert";
    boxProgram->fragmentShaderFilename = "res/shaders/box.frag";
    boxProgram->includeForSerialization = false;

    selectionMaskProgram = resourceManager->createShaderProgram();
    selectionMaskProgram->name = "Selection mask";
    selectionMaskProgram->vertexShaderFilename = "res/shaders/selection_mask.vert";
    selectionMaskProgram->fragmentShaderFilename = "res/shaders/selection_mask.frag";
    selectionMaskProgram->includeForSerialization = false;

    selectionOutlineProgram = resourceManager->createShaderProgram();
    selectionOutlineProgram->name = "Selection outline";
    selectionOutlineProgram->vertexShaderFilename = "res/shaders/selection_outline.vert";
    selectionOutlineProgram->fragmentShaderFilename = "res/shaders/selection_outline.frag";
    selectionOutlineProgram->includeForSerialization = false;

    gridProgram = resourceManager->createShaderProgram();
    gridProgram->name = "Grid";
    gridProgram->vertexShaderFilename = "res/shaders/grid.vert";
    gridProgram->fragmentShaderFilename = "res/shaders/grid.frag";
    gridProgram->includeForSerialization = false;

    blitProgram = resourceManager->createShaderProgram();
    blitProgram->name = "Blit";
    blitProgram->vertexShaderFilename = "res/shaders/blit.vert";
    blitProgram->fragmentShaderFilename = "res/shaders/blit.frag";
    blitProgram->includeForSerialization = false;


    // Create FBO

    fbo = new FramebufferObject;
    fbo->create();
}

void DeferredRenderer::finalize()
{
    fbo->destroy();
    delete fbo;
}

void DeferredRenderer::resize(int w, int h)
{
    OpenGLErrorGuard guard("ForwardRenderer::resize()");

    // Get size
    viewportWidth = w;
    viewportHeight = h;


    // Regenerate textures

    if (rt0 != 0) gl->glDeleteTextures(1, &rt0);
    gl->glGenTextures(1, &rt0);
    gl->glBindTexture(GL_TEXTURE_2D, rt0);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    gl->glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    if (rt1 != 0) gl->glDeleteTextures(1, &rt1);
    gl->glGenTextures(1, &rt1);
    gl->glBindTexture(GL_TEXTURE_2D, rt1);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    gl->glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    if (rt2 != 0) gl->glDeleteTextures(1, &rt2);
    gl->glGenTextures(1, &rt2);
    gl->glBindTexture(GL_TEXTURE_2D, rt2);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    gl->glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    if (rt3 != 0) gl->glDeleteTextures(1, &rt3);
    gl->glGenTextures(1, &rt3);
    gl->glBindTexture(GL_TEXTURE_2D, rt3);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    gl->glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, w, h, 0, GL_RGBA, GL_FLOAT, nullptr);

    if (rt4 != 0) gl->glDeleteTextures(1, &rt4);
    gl->glGenTextures(1, &rt4);
    gl->glBindTexture(GL_TEXTURE_2D, rt4);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    gl->glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, w, h, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);

    if (rt5 != 0) gl->glDeleteTextures(1, &rt5);
    gl->glGenTextures(1, &rt5);
    gl->glBindTexture(GL_TEXTURE_2D, rt5);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    gl->glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, w, h, 0, GL_RGBA, GL_FLOAT, nullptr);


    // Attach textures to the fbo

    fbo->bind();
    fbo->addColorAttachment(rt0, 0);
    fbo->addColorAttachment(rt1, 1);
    fbo->addColorAttachment(rt2, 2);
    fbo->addColorAttachment(rt3, 3);
    fbo->addColorAttachment(rt5, 5);
    fbo->addDepthAttachment(rt4);
    fbo->checkStatus();
    fbo->release();


    // Reset texture identifiers

    setTexture("Albedo / Occlusion", rt0);
    setTexture("Specular / Roughness", rt1);
    setTexture("Normals", rt2);
    setTexture("Final render", rt3);
    setTexture("Depth", rt4);
    setTexture("Temp 1", rt5);
}

void DeferredRenderer::render(Camera *camera)
{
    OpenGLErrorGuard guard("ForwardRenderer::render()");

    fbo->bind();

    // Passes
    passMeshes(camera);
    passLights(camera);
    passBackground(camera);
    passSelectionOutline(camera);
    passGrid(camera);

    fbo->release();

    passBlit();
}

extern int g_MaxSubmeshes;

void DeferredRenderer::passMeshes(Camera *camera)
{
    GLenum drawBuffers[] = {
        GL_COLOR_ATTACHMENT0,
        GL_COLOR_ATTACHMENT1,
        GL_COLOR_ATTACHMENT2,
        GL_COLOR_ATTACHMENT3
    };
    gl->glDrawBuffers(4, drawBuffers);

    OpenGLState glState;
    glState.depthTest = true;
    glState.apply();

    gl->glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    gl->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    QOpenGLShaderProgram &program = materialProgram->program;

    if (program.bind())
    {
        QMatrix4x4 viewMatrix = camera->viewMatrix;
        program.setUniformValue("viewMatrix", viewMatrix);
        program.setUniformValue("projectionMatrix", camera->projectionMatrix);
        program.setUniformValue("eyeWorldspace", QVector3D(camera->worldMatrix * QVector4D(0.0, 0.0, 0.0, 1.0)));

        QVector<MeshRenderer*> meshRenderers;
        QVector<LightSource*> lightSources;

        // Get components
        for (auto entity : scene->entities)
        {
            if (!entity->active) continue;
            if (entity->meshRenderer != nullptr) { meshRenderers.push_back(entity->meshRenderer); }
            if (entity->lightSource != nullptr) { lightSources.push_back(entity->lightSource); }
        }

        // Meshes
        for (auto meshRenderer : meshRenderers)
        {
            auto mesh = meshRenderer->mesh;

            if (mesh != nullptr)
            {
                QMatrix4x4 worldMatrix = meshRenderer->entity->transform->matrix();
                QMatrix3x3 normalMatrix = worldMatrix.normalMatrix();

                program.setUniformValue("worldMatrix", worldMatrix);
                program.setUniformValue("normalMatrix", normalMatrix);

                int materialIndex = 0;
                for (auto submesh : mesh->submeshes)
                {
                    if (materialIndex >= g_MaxSubmeshes) break;
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
                    program.setUniformValue("tiling", material->tiling);
                    SEND_TEXTURE("albedoTexture", material->albedoTexture, resourceManager->texWhite, 0);
                    SEND_TEXTURE("emissiveTexture", material->emissiveTexture, resourceManager->texBlack, 1);
                    SEND_TEXTURE("specularTexture", material->specularTexture, resourceManager->texBlack, 2);
                    SEND_TEXTURE("normalTexture", material->normalsTexture, resourceManager->texNormal, 3);
                    SEND_TEXTURE("bumpTexture", material->bumpTexture, resourceManager->texWhite, 4);

                    submesh->draw();
                }
            }
        }

        // Light spheres
        for (auto lightSource : lightSources)
        {
            QMatrix4x4 worldMatrix = lightSource->entity->transform->matrix();
            QMatrix3x3 normalMatrix = worldMatrix.normalMatrix();
            worldMatrix.scale(0.1f, 0.1f, 0.1f);

            program.setUniformValue("worldMatrix", worldMatrix);
            program.setUniformValue("normalMatrix", normalMatrix);

            // Send the material to the shader
            Material *material = resourceManager->materialLight;
            program.setUniformValue("albedo", material->albedo);
            program.setUniformValue("emissive", material->emissive);
            program.setUniformValue("smoothness", material->smoothness);
            SEND_TEXTURE("albedoTexture", material->albedoTexture, resourceManager->texWhite, 0);
            SEND_TEXTURE("emissiveTexture", material->emissiveTexture, resourceManager->texBlack, 1);
            SEND_TEXTURE("specularTexture", material->specularTexture, resourceManager->texBlack, 2);
            SEND_TEXTURE("normalTexture", material->normalsTexture, resourceManager->texNormal, 3);
            SEND_TEXTURE("bumpTexture", material->bumpTexture, resourceManager->texWhite, 4);

            resourceManager->sphere->submeshes[0]->draw();
        }

        program.release();
    }
}

void DeferredRenderer::passLights(Camera *camera)
{
    GLenum drawBuffers[] = { GL_COLOR_ATTACHMENT3 };
    gl->glDrawBuffers(1, drawBuffers);

    OpenGLState glState;
    glState.depthTest = true;
    glState.depthWrite = false;
    glState.blending = true;
    glState.blendFuncDst = GL_ONE;
    glState.blendFuncSrc = GL_ONE;
    glState.apply();

    QOpenGLShaderProgram &program = lightingProgram->program;

    if (program.bind())
    {
        // Input render targets
        program.setUniformValue("rt0", 0); gl->glActiveTexture(GL_TEXTURE0 + 0); gl->glBindTexture(GL_TEXTURE_2D, rt0);
        program.setUniformValue("rt1", 1); gl->glActiveTexture(GL_TEXTURE0 + 1); gl->glBindTexture(GL_TEXTURE_2D, rt1);
        program.setUniformValue("rt2", 2); gl->glActiveTexture(GL_TEXTURE0 + 2); gl->glBindTexture(GL_TEXTURE_2D, rt2);
        program.setUniformValue("rt4", 3); gl->glActiveTexture(GL_TEXTURE0 + 3); gl->glBindTexture(GL_TEXTURE_2D, rt4);

        // Viewport parameters
        program.setUniformValue("viewportSize", QVector2D(viewportWidth, viewportHeight));

        // Camera parameters
        QMatrix4x4 viewMatrix = camera->viewMatrix;
        QMatrix4x4 viewMatrixInv = viewMatrix.inverted();
        QMatrix4x4 projectionMatrix = camera->projectionMatrix;
        QVector4D viewportParams = camera->getLeftRightBottomTop();
        QVector3D cameraPosition = QVector3D(camera->worldMatrix * QVector4D(0.0, 0.0, 0.0, 1.0));
        program.setUniformValue("viewMatrix", viewMatrix);
        program.setUniformValue("viewMatrixInv", viewMatrixInv);
        program.setUniformValue("projectionMatrix", projectionMatrix);
        program.setUniformValue("left", viewportParams.x());
        program.setUniformValue("right", viewportParams.y());
        program.setUniformValue("bottom", viewportParams.z());
        program.setUniformValue("top", viewportParams.w());
        program.setUniformValue("znear", camera->znear);
        program.setUniformValue("zfar", camera->zfar);
        program.setUniformValue("eyeWorldspace", cameraPosition);

        // For all lights...
        for (auto entity : scene->entities)
        {
            if (entity->active == false) continue;
            if (entity->lightSource == nullptr) continue;
            auto lightSource = entity->lightSource;

            // World matrix
            QMatrix4x4 worldMatrix = lightSource->entity->transform->matrix();
            QMatrix4x4 scale; scale.scale(lightSource->range);
            worldMatrix = worldMatrix * scale;
            program.setUniformValue("worldMatrix", worldMatrix);

            // Light parameters
            const QVector3D lightPosition = QVector3D(entity->transform->matrix() * QVector4D(0.0, 0.0, 0.0, 1.0));
            const QVector3D lightDirection = QVector3D(entity->transform->matrix() * QVector4D(0.0, 1.0, 0.0, 0.0));
            program.setUniformValue("lightType", (int)lightSource->type);
            program.setUniformValue("lightPosition", lightPosition);
            program.setUniformValue("lightDirection", lightDirection);
            program.setUniformValue("lightRange", lightSource->range);
            program.setUniformValue("lightColor", QVector3D(lightSource->color.redF(),
                                                            lightSource->color.greenF(),
                                                            lightSource->color.blueF()) * lightSource->intensity);

            // Render quad or sphere?
            const bool renderQuad = lightSource->type ==
                    LightSource::Type::Directional ||
                    (lightSource->type == LightSource::Type::Point &&
                     lightSource->range > cameraPosition.distanceToPoint(lightPosition) - camera->znear);
            if (renderQuad)
            {
                program.setUniformValue("lightQuad", 1);
                resourceManager->quad->submeshes[0]->draw();
            }
            else
            {
                program.setUniformValue("lightQuad", 0);
                resourceManager->sphere->submeshes[0]->draw();
            }
        }

        program.release();
    }
}

void DeferredRenderer::passBackground(Camera *camera)
{
    GLenum drawBuffers[] = { GL_COLOR_ATTACHMENT3 };
    gl->glDrawBuffers(1, drawBuffers);

    OpenGLState glState;
    glState.depthTest = true;
    glState.depthFunc = GL_LEQUAL;
    glState.apply();

    QOpenGLShaderProgram &program = backgroundProgram->program;

    if (program.bind())
    {
        program.setUniformValue("backgroundColor", scene->backgroundColor);

        resourceManager->quad->submeshes[0]->draw();

        program.release();
    }
}

void DeferredRenderer::passSelectionOutline(Camera *camera)
{
    // Avoid this pass in case there is no selection
    if (selection->count < 1) return;

//    GLenum drawBuffers[] = { GL_COLOR_ATTACHMENT3 };
//    gl->glDrawBuffers(1, drawBuffers);

//    OpenGLState glState;
//    glState.depthTest = true;
//    glState.blending = true;
//    glState.blendFuncSrc = GL_SRC_ALPHA;
//    glState.blendFuncDst = GL_ONE_MINUS_SRC_ALPHA;
//    glState.apply();

//    QOpenGLShaderProgram &program = boxProgram->program;

//    if (program.bind())
//    {
//        // Camera parameters
//        program.setUniformValue("viewMatrix", camera->viewMatrix);
//        program.setUniformValue("projectionMatrix", camera->projectionMatrix);

//        for (int i = 0; i < selection->count; ++i)
//        {
//            auto entity = selection->entities[i];
//            if (entity->meshRenderer == nullptr || entity->meshRenderer->mesh == nullptr) continue;
//            program.setUniformValue("worldMatrix", entity->transform->matrix());
//            program.setUniformValue("boundsMin", entity->meshRenderer->mesh->bounds.min);
//            program.setUniformValue("boundsMax", entity->meshRenderer->mesh->bounds.max);
//            resourceManager->unitCubeGrid->submeshes[0]->draw(GL_LINES);
//        }
//    }

    // Selection mask
    {
        GLenum drawBuffers[] = { GL_COLOR_ATTACHMENT5 };
        gl->glDrawBuffers(1, drawBuffers);

        gl->glClearColor(0.0, 0.0, 0.0, 0.0);
        gl->glClear(GL_COLOR_BUFFER_BIT);

        OpenGLState glState;
        glState.depthTest = true;
        glState.depthWrite = false;
        glState.depthFunc = GL_LEQUAL;
        glState.apply();

        QOpenGLShaderProgram &program = selectionMaskProgram->program;

        if (program.bind())
        {
            // Camera parameters
            program.setUniformValue("viewMatrix", camera->viewMatrix);
            program.setUniformValue("projectionMatrix", camera->projectionMatrix);

            for (int i = 0; i < selection->count; ++i)
            {
                auto entity = selection->entities[i];
                if (entity->meshRenderer == nullptr) continue;
                if (entity->meshRenderer->mesh == nullptr) continue;
                auto mesh = entity->meshRenderer->mesh;

                program.setUniformValue("worldMatrix", entity->transform->matrix());

                for (auto submesh : mesh->submeshes)
                {
                    submesh->draw();
                }
            }

            program.release();
        }
    }

    // Selection outline
    {
        GLenum drawBuffers[] = { GL_COLOR_ATTACHMENT3 };
        gl->glDrawBuffers(1, drawBuffers);

        OpenGLState glState;
        glState.depthTest = false;
        glState.blending = true;
        glState.blendFuncSrc = GL_SRC_ALPHA;
        glState.blendFuncDst = GL_ONE_MINUS_SRC_ALPHA;
        glState.apply();

        QOpenGLShaderProgram &program = selectionOutlineProgram->program;

        if (program.bind())
        {
            program.setUniformValue("mask", 0);
            gl->glActiveTexture(GL_TEXTURE0);
            gl->glBindTexture(GL_TEXTURE_2D, rt5);

            resourceManager->quad->submeshes[0]->draw();

            program.release();
        }
    }
}

void DeferredRenderer::passGrid(Camera *camera)
{
    GLenum drawBuffers[] = { GL_COLOR_ATTACHMENT3 };
    gl->glDrawBuffers(1, drawBuffers);

    OpenGLState glState;
    glState.depthTest = true;
    glState.blending = true;
    glState.blendFuncSrc = GL_SRC_ALPHA;
    glState.blendFuncDst = GL_ONE_MINUS_SRC_ALPHA;
    glState.apply();

    QOpenGLShaderProgram &program = gridProgram->program;

    if (program.bind())
    {
        QVector4D cameraParameters = camera->getLeftRightBottomTop();
        program.setUniformValue("left", cameraParameters.x());
        program.setUniformValue("right", cameraParameters.y());
        program.setUniformValue("bottom", cameraParameters.z());
        program.setUniformValue("top", cameraParameters.w());
        program.setUniformValue("znear", camera->znear);
        program.setUniformValue("worldMatrix", camera->worldMatrix);
        program.setUniformValue("viewMatrix", camera->viewMatrix);
        program.setUniformValue("projectionMatrix", camera->projectionMatrix);

        resourceManager->quad->submeshes[0]->draw();

        program.release();
    }
}

void DeferredRenderer::passBlit()
{
    OpenGLState::reset();

    QOpenGLShaderProgram &program = blitProgram->program;

    if (program.bind())
    {
        program.setUniformValue("colorTexture", 0);
        gl->glActiveTexture(GL_TEXTURE0);
        gl->glBindTexture(GL_TEXTURE_2D, shownTexture());

        resourceManager->quad->submeshes[0]->draw();

        program.release();
    }
}
