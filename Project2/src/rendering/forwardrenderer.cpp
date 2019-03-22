#include "forwardrenderer.h"
#include "ecs/scene.h"
#include "ecs/camera.h"
#include "resources/material.h"
#include "resources/mesh.h"
#include "resources/texture.h"
#include "resources/shaderprogram.h"
#include "opengl/functions.h"
#include "globals.h"
#include <QVector>
#include <QVector3D>
#include <QOpenGLShaderProgram>


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

ForwardRenderer::ForwardRenderer()
{

}

void ForwardRenderer::resize(int w, int h)
{

}

void ForwardRenderer::render(Camera *camera)
{
    // Clear color
    gl->glClearDepth(1.0);
    gl->glClearColor(0.7f, 0.8f, 1.0f, 1.0f);
    gl->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Backface culling and z-test
    gl->glEnable(GL_CULL_FACE);
    gl->glCullFace(GL_BACK);
    gl->glEnable(GL_DEPTH_TEST);

    QOpenGLShaderProgram &program = resourceManager->forwardShading->program;

    if (program.bind())
    {
        program.setUniformValue("viewMatrix", camera->viewMatrix);
        program.setUniformValue("projectionMatrix", camera->projectionMatrix);

        sendLightsToProgram(program, camera->viewMatrix);

        for (auto entity : scene->entities)
        {
            auto meshRenderer = entity->meshRenderer;

            if (meshRenderer != nullptr)
            {
                auto mesh = meshRenderer->mesh;

                if (mesh != nullptr)
                {
                    QMatrix4x4 worldMatrix = entity->transform->matrix();
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
        }

        // Render lights
        for (auto entity : scene->entities)
        {
            auto lightSource = entity->lightSource;

            if (lightSource != nullptr)
            {
                QMatrix4x4 worldMatrix = entity->transform->matrix();
                QMatrix4x4 scaleMatrix; scaleMatrix.scale(0.1, 0.1, 0.1);
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
}
