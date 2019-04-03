#include "forwardrenderer.h"
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

ForwardRenderer::ForwardRenderer() :
    fboColor(QOpenGLTexture::Target2D),
    fboDepth(QOpenGLTexture::Target2D)
{
    fbo = nullptr;

    // List of textures
    addTexture("Final render");
    addTexture("Depth");
    addTexture("Normals");
}

ForwardRenderer::~ForwardRenderer()
{
    delete fbo;
}

void ForwardRenderer::initialize()
{
    OpenGLErrorGuard guard("ForwardRenderer::initialize()");

    // Create programs

    objectsProgram = resourceManager->createShaderProgram();
    objectsProgram->name = "Forward shading";
    objectsProgram->vertexShaderFilename = "res/shaders/forward_shading.vert";
    objectsProgram->fragmentShaderFilename = "res/shaders/forward_shading.frag";
    objectsProgram->includeForSerialization = false;

    terrainProgram = resourceManager->createShaderProgram();
    terrainProgram->name = "Forward shading (terrain)";
    terrainProgram->vertexShaderFilename = "res/shaders/forward_shading_terrain.vert";
    terrainProgram->fragmentShaderFilename = "res/shaders/forward_shading_terrain.frag";
    terrainProgram->includeForSerialization = false;

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

void ForwardRenderer::finalize()
{
    fbo->destroy();
    delete fbo;
}

void ForwardRenderer::resize(int w, int h)
{
    OpenGLErrorGuard guard("ForwardRenderer::resize()");

    // Regenerate textures

    if (fboColor == 0) gl->glDeleteTextures(1, &fboColor);
    gl->glGenTextures(1, &fboColor);
    gl->glBindTexture(GL_TEXTURE_2D, fboColor);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    gl->glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    if (fboDepth == 0) gl->glDeleteTextures(1, &fboDepth);
    gl->glGenTextures(1, &fboDepth);
    gl->glBindTexture(GL_TEXTURE_2D, fboDepth);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    gl->glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, w, h, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);


    // Attach textures to the fbo

    fbo->bind();
    fbo->addColorAttachment(fboColor, 0);
    fbo->addDepthAttachment(fboDepth);
    fbo->checkStatus();
    fbo->release();


    // Reset texture identifiers

    setTexture("Final render", fboColor);
    setTexture("Depth", fboDepth);
    setTexture("Normals", resourceManager->texNormal->textureId());
}

void ForwardRenderer::render(Camera *camera)
{
    OpenGLErrorGuard guard("ForwardRenderer::render()");

    fbo->bind();

    // Clear color
    gl->glClearDepth(1.0);
    gl->glClearColor(0.4f, 0.4f, 0.5f, 1.0f);
    gl->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Passes
    passMeshes(camera);
    passTerrains(camera);
    passGrid(camera);

    gl->glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    fbo->release();

    passBlit();
}

void ForwardRenderer::passMeshes(Camera *camera)
{
    QOpenGLShaderProgram &program = objectsProgram->program;

    if (program.bind())
    {
        program.setUniformValue("viewMatrix", camera->viewMatrix);
        program.setUniformValue("projectionMatrix", camera->projectionMatrix);

        sendLightsToProgram(program, camera->viewMatrix);

        QVector<MeshRenderer*> meshRenderers;
        QVector<LightSource*> lightSources;

        // Get components
        for (auto entity : scene->entities)
        {
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
                QMatrix4x4 worldViewMatrix = camera->viewMatrix * worldMatrix;
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
            QMatrix4x4 scaleMatrix; scaleMatrix.scale(0.1f, 0.1f, 0.1f);
            QMatrix4x4 worldViewMatrix = camera->viewMatrix * worldMatrix * scaleMatrix;
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

        program.release();
    }
}

void ForwardRenderer::passTerrains(Camera *camera)
{
    QOpenGLShaderProgram &program = terrainProgram->program;

    if (program.bind())
    {
        // Render terrains
        for (auto entity : scene->entities)
        {
            program.setUniformValue("viewMatrix", camera->viewMatrix);
            program.setUniformValue("projectionMatrix", camera->projectionMatrix);

            sendLightsToProgram(program, camera->viewMatrix);

            auto terrainRenderer = entity->terrainRenderer;

            if (terrainRenderer != nullptr)
            {
                auto mesh = terrainRenderer->mesh;

                if (mesh != nullptr)
                {
                    QMatrix4x4 worldMatrix = entity->transform->matrix();
                    QMatrix4x4 worldViewMatrix = camera->viewMatrix * worldMatrix;
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
        }

        program.release();
    }
}

void ForwardRenderer::passGrid(Camera *camera)
{
    gl->glEnable(GL_BLEND);
    gl->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

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

        for (auto submesh : resourceManager->quad->submeshes)
        {
            submesh->draw();
        }
    }

    gl->glDisable(GL_BLEND);
}

void ForwardRenderer::passBlit()
{
    gl->glDisable(GL_DEPTH_TEST);

    QOpenGLShaderProgram &program = blitProgram->program;

    if (program.bind())
    {
        program.setUniformValue("colorTexture", 0);
        gl->glActiveTexture(GL_TEXTURE0);
        gl->glBindTexture(GL_TEXTURE_2D, shownTexture());

        resourceManager->quad->submeshes[0]->draw();
    }

    gl->glEnable(GL_DEPTH_TEST);
}
