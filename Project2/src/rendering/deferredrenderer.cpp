#include "deferredrenderer.h"
#include "ecs/scene.h"
#include "ecs/camera.h"
#include "resources/material.h"
#include "resources/mesh.h"
#include "resources/texture.h"
#include "resources/texturecube.h"
#include "resources/shaderprogram.h"
#include "resources/resourcemanager.h"
#include "framebufferobject.h"
#include "gl.h"
#include "globals.h"
#include <QVector>
#include <QVector3D>
#include <QOpenGLShaderProgram>
#include <QOpenGLTexture>
#include <QRandomGenerator>


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
    OpenGLErrorGuard guard("DeferredRenderer::initialize()");

    // Create programs

    equirectangularToCubemapProgram = resourceManager->createShaderProgram();
    equirectangularToCubemapProgram->name = "Equirectangular to cubemap";
    equirectangularToCubemapProgram->vertexShaderFilename = "res/shaders/equirectangular_to_cubemap.vert";
    equirectangularToCubemapProgram->fragmentShaderFilename = "res/shaders/equirectangular_to_cubemap.frag";
    equirectangularToCubemapProgram->includeForSerialization = false;

    irradianceProgram = resourceManager->createShaderProgram();
    irradianceProgram->name = "Irradiance generator";
    irradianceProgram->vertexShaderFilename = "res/shaders/irradiance.vert";
    irradianceProgram->fragmentShaderFilename = "res/shaders/irradiance.frag";
    irradianceProgram->includeForSerialization = false;

    materialProgram = resourceManager->createShaderProgram();
    materialProgram->name = "Deferred material";
    materialProgram->vertexShaderFilename = "res/shaders/deferred_material.vert";
    materialProgram->fragmentShaderFilename = "res/shaders/deferred_material.frag";
    materialProgram->includeForSerialization = false;

    ssaoProgram = resourceManager->createShaderProgram();
    ssaoProgram->name = "SSAO";
    ssaoProgram->vertexShaderFilename = "res/shaders/ssao.vert";
    ssaoProgram->fragmentShaderFilename = "res/shaders/ssao.frag";
    ssaoProgram->includeForSerialization = false;

    ssaoBlurProgram = resourceManager->createShaderProgram();
    ssaoBlurProgram->name = "SSAO Blur";
    ssaoBlurProgram->vertexShaderFilename = "res/shaders/ssao_blur.vert";
    ssaoBlurProgram->fragmentShaderFilename = "res/shaders/ssao_blur.frag";
    ssaoBlurProgram->includeForSerialization = false;

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

    blurProgram = resourceManager->createShaderProgram();
    blurProgram->name = "Blur";
    blurProgram->vertexShaderFilename = "res/shaders/blur.vert";
    blurProgram->fragmentShaderFilename = "res/shaders/blur.frag";
    blurProgram->includeForSerialization = false;

    blitProgram = resourceManager->createShaderProgram();
    blitProgram->name = "Blit";
    blitProgram->vertexShaderFilename = "res/shaders/blit.vert";
    blitProgram->fragmentShaderFilename = "res/shaders/blit.frag";
    blitProgram->includeForSerialization = false;

    // Generation of random samples for SSAO
    std::uniform_real_distribution<float> randomFloats(0.0, 1.0);
    std::default_random_engine generator;
    for (unsigned int i = 0; i < 64; ++i)
    {
        QVector3D sample;

        do
        {
            sample = QVector3D(
                randomFloats(generator) * 2.0 - 1.0,
                randomFloats(generator) * 2.0 - 1.0,
                randomFloats(generator) * 0.8 + 0.2
            );
            sample.normalize();
            sample *= randomFloats(generator);
            float scale = (float)i / 64.0;
            scale = 0.1f + 0.9f * scale * scale; // lerp(0.1, 1.0, scale*scale)
            sample *= scale;
        } while (sample.length() > 1.0 ||
                 QVector3D::dotProduct(QVector3D(0, 0, 1), sample.normalized()) < 0.3f);
        ssaoKernel.push_back(sample);
    }

    // Generation of a 4x4 texture of random vectors
    QVector<QVector3D> ssaoNoise;
    for (unsigned int i = 0; i < 16; i++)
    {
        QVector3D noise(
            randomFloats(generator) * 2.0 - 1.0,
            randomFloats(generator) * 2.0 - 1.0,
            0.0f);
        ssaoNoise.push_back(noise);
    }
    gl->glGenTextures(1, &ssaoNoiseTex);
    gl->glBindTexture(GL_TEXTURE_2D, ssaoNoiseTex);
    gl->glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, 4, 4, 0, GL_RGB, GL_FLOAT, &ssaoNoise[0]);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    // Create FBO

    fbo = new FramebufferObject;
    fbo->create();
}

void DeferredRenderer::finalize()
{
    fbo->destroy();
    delete fbo;

    GLuint textures[] = { rt0, rt1, rt2, rt3, rt4, rt5, ssaoNoiseTex };
    gl->glDeleteTextures(7, textures);
}

void DeferredRenderer::resize(int w, int h)
{
    OpenGLErrorGuard guard("DeferredRenderer::resize()");

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
    gl->glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

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
    OpenGLErrorGuard guard("DeferredRenderer::render()");

    passEnvironments();

    fbo->bind();

    // Passes
    passMeshes(camera);
    passSSAO(camera);
    passSSAOBlur();
    passLights(camera);
    passBackground(camera);
    passSelectionOutline(camera);
    passGrid(camera);
//    passMotionBlur(camera);

    fbo->release();

    passBlit();
}

static Environment *g_Environment = nullptr;

void DeferredRenderer::passEnvironments()
{
    OpenGLErrorGuard guard("DeferredRenderer::passEnvironments()");

    for (auto entity : scene->entities)
    {
        auto environment = entity->environment;

        if (environment != nullptr && environment->needsProcessing)
        {
            if (environment->texture != nullptr)
            {
                // Create temporary FBO
                unsigned int captureFBO, captureRBO;
                gl->glGenFramebuffers(1, &captureFBO);
                gl->glGenRenderbuffers(1, &captureRBO);
                gl->glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
                gl->glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
                gl->glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, environment->environmentMap->resolution, environment->environmentMap->resolution);
                gl->glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, captureRBO);

                // Transformation matrices
                QMatrix4x4 captureProjection;
                captureProjection.perspective(90.0f, 1.0f, 0.1f, 10.0f);
                QMatrix4x4 captureViews[6];
                captureViews[0].lookAt(QVector3D(0.0f, 0.0f, 0.0f), QVector3D( 1.0f,  0.0f,  0.0f), QVector3D(0.0f, -1.0f,  0.0f));
                captureViews[1].lookAt(QVector3D(0.0f, 0.0f, 0.0f), QVector3D(-1.0f,  0.0f,  0.0f), QVector3D(0.0f, -1.0f,  0.0f));
                captureViews[2].lookAt(QVector3D(0.0f, 0.0f, 0.0f), QVector3D( 0.0f,  1.0f,  0.0f), QVector3D(0.0f,  0.0f,  1.0f));
                captureViews[3].lookAt(QVector3D(0.0f, 0.0f, 0.0f), QVector3D( 0.0f, -1.0f,  0.0f), QVector3D(0.0f,  0.0f, -1.0f));
                captureViews[4].lookAt(QVector3D(0.0f, 0.0f, 0.0f), QVector3D( 0.0f,  0.0f,  1.0f), QVector3D(0.0f, -1.0f,  0.0f));
                captureViews[5].lookAt(QVector3D(0.0f, 0.0f, 0.0f), QVector3D( 0.0f,  0.0f, -1.0f), QVector3D(0.0f, -1.0f,  0.0f));

                // convert HDR equirectangular environment map to cubemap equivalent
                QOpenGLShaderProgram &program = equirectangularToCubemapProgram->program;

                if (program.bind())
                {
                    OpenGLState glState;
                    glState.faceCulling = false;
                    glState.apply();

                    program.setUniformValue("equirectangularMap", 0);
                    program.setUniformValue("projectionMatrix", captureProjection);

                    gl->glActiveTexture(GL_TEXTURE0);
                    gl->glBindTexture(GL_TEXTURE_2D, environment->texture->textureId());

                    gl->glViewport(0, 0, environment->environmentMap->resolution, environment->environmentMap->resolution);
                    gl->glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
                    for (unsigned int i = 0; i < 6; ++i)
                    {
                        program.setUniformValue("viewMatrix", captureViews[i]);
                        gl->glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                                               GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, environment->environmentMap->textureId(), 0);
                        gl->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

                        resourceManager->cube->submeshes[0]->draw();
                    }

                    gl->glBindTexture(GL_TEXTURE_CUBE_MAP, environment->environmentMap->textureId());
                    gl->glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

                    program.release();
                }

                gl->glDeleteFramebuffers(1, &captureFBO);
                gl->glDeleteRenderbuffers(1, &captureRBO);
            }

            if (environment->texture != nullptr)
            {
                // Create temporary FBO
                unsigned int captureFBO, captureRBO;
                gl->glGenFramebuffers(1, &captureFBO);
                gl->glGenRenderbuffers(1, &captureRBO);
                gl->glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
                gl->glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
                gl->glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, environment->irradianceMap->resolution, environment->irradianceMap->resolution);
                gl->glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, captureRBO);

                // Transformation matrices
                QMatrix4x4 captureProjection;
                captureProjection.perspective(90.0f, 1.0f, 0.1f, 10.0f);
                QMatrix4x4 captureViews[6];
                captureViews[0].lookAt(QVector3D(0.0f, 0.0f, 0.0f), QVector3D( 1.0f,  0.0f,  0.0f), QVector3D(0.0f, -1.0f,  0.0f));
                captureViews[1].lookAt(QVector3D(0.0f, 0.0f, 0.0f), QVector3D(-1.0f,  0.0f,  0.0f), QVector3D(0.0f, -1.0f,  0.0f));
                captureViews[2].lookAt(QVector3D(0.0f, 0.0f, 0.0f), QVector3D( 0.0f,  1.0f,  0.0f), QVector3D(0.0f,  0.0f,  1.0f));
                captureViews[3].lookAt(QVector3D(0.0f, 0.0f, 0.0f), QVector3D( 0.0f, -1.0f,  0.0f), QVector3D(0.0f,  0.0f, -1.0f));
                captureViews[4].lookAt(QVector3D(0.0f, 0.0f, 0.0f), QVector3D( 0.0f,  0.0f,  1.0f), QVector3D(0.0f, -1.0f,  0.0f));
                captureViews[5].lookAt(QVector3D(0.0f, 0.0f, 0.0f), QVector3D( 0.0f,  0.0f, -1.0f), QVector3D(0.0f, -1.0f,  0.0f));

                // create HDR irradiance cubemap
                QOpenGLShaderProgram &program2 = irradianceProgram->program;

                if (program2.bind())
                {
                    OpenGLState glState;
                    glState.faceCulling = false;
                    glState.apply();

                    program2.setUniformValue("environmentMap", 0);
                    program2.setUniformValue("projectionMatrix", captureProjection);

//                    program2.setUniformValue("numTangentSamples", NUM_TANGENT_SAMPLES);
//                    program2.setUniformValueArray("tangentSamples", &tangentSamples[0], NUM_TANGENT_SAMPLES);

                    gl->glActiveTexture(GL_TEXTURE0);
                    gl->glBindTexture(GL_TEXTURE_CUBE_MAP, environment->environmentMap->textureId());

                    gl->glViewport(0, 0, environment->irradianceMap->resolution, environment->irradianceMap->resolution);
                    gl->glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
                    for (unsigned int i = 0; i < 6; ++i)
                    {
                        program2.setUniformValue("viewMatrix", captureViews[i]);
                        gl->glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                                               GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, environment->irradianceMap->textureId(), 0);
                        gl->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

                        resourceManager->cube->submeshes[0]->draw();
                    }

                    program2.release();
                }

                gl->glViewport(0, 0, viewportWidth, viewportHeight);

                gl->glDeleteFramebuffers(1, &captureFBO);
                gl->glDeleteRenderbuffers(1, &captureRBO);

                entity->environment->needsProcessing = false;

                g_Environment = environment; // Chapucilla
                return;
            }
        }
    }
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

                    // Switch between texture / no-texture
                    float metalness = material->metalness;
                    if (material->specularTexture != nullptr) {
                        metalness = 1.0f;
                    }

                    // Send the material to the shader
                    program.setUniformValue("albedo", material->albedo);
                    program.setUniformValue("emissive", material->emissive);
                    program.setUniformValue("specular", material->specular);
                    program.setUniformValue("smoothness", material->smoothness);
                    program.setUniformValue("metalness", metalness);
                    program.setUniformValue("bumpiness", material->bumpiness);
                    program.setUniformValue("tiling", material->tiling);
                    SEND_TEXTURE("albedoTexture", material->albedoTexture, resourceManager->texWhite, 0);
                    SEND_TEXTURE("emissiveTexture", material->emissiveTexture, resourceManager->texBlack, 1);
                    SEND_TEXTURE("specularTexture", material->specularTexture, resourceManager->texWhite, 2);
                    SEND_TEXTURE("normalTexture", material->normalsTexture, resourceManager->texNormal, 3);
                    SEND_TEXTURE("bumpTexture", material->bumpTexture, resourceManager->texWhite, 4);

                    submesh->draw();
                }
            }
        }

        // Light spheres
        if (scene->renderLightSources)
        {
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
        }

#undef SEND_TEXTURE

        program.release();
    }
}

void DeferredRenderer::passSSAO(Camera *camera)
{
    OpenGLErrorGuard guard("DeferredRenderer::passSSAO()");

    gl->glDrawBuffer(GL_COLOR_ATTACHMENT5);

    OpenGLState glState;
    glState.depthTest = false;
    glState.apply();

    QOpenGLShaderProgram &program = ssaoProgram->program;

    if (program.bind())
    {
        // Viewport parameters
        program.setUniformValue("viewportSize", QVector2D(viewportWidth, viewportHeight));

        // Camera parameters
        QVector4D viewportParams = camera->getLeftRightBottomTop();
        program.setUniformValue("left", viewportParams.x());
        program.setUniformValue("right", viewportParams.y());
        program.setUniformValue("bottom", viewportParams.z());
        program.setUniformValue("top", viewportParams.w());
        program.setUniformValue("znear", camera->znear);
        program.setUniformValue("zfar", camera->zfar);
        program.setUniformValue("viewMatrix", camera->viewMatrix);
        program.setUniformValue("projectionMatrix", camera->projectionMatrix);

        // Normal map
        gl->glActiveTexture(GL_TEXTURE0);
        gl->glBindTexture(GL_TEXTURE_2D, rt2);
        program.setUniformValue("normalMap", 0);

        // Depth map
        gl->glActiveTexture(GL_TEXTURE1);
        gl->glBindTexture(GL_TEXTURE_2D, rt4);
        program.setUniformValue("depthMap", 1);

        // Noise map
        gl->glActiveTexture(GL_TEXTURE2);
        gl->glBindTexture(GL_TEXTURE_2D, ssaoNoiseTex);
        program.setUniformValue("noiseMap", 2);

        // SSAO kernel
        program.setUniformValueArray("samples", &ssaoKernel[0], ssaoKernel.size());

        resourceManager->quad->submeshes[0]->draw();

        program.release();
    }
}

void DeferredRenderer::passSSAOBlur()
{
    gl->glDrawBuffer(GL_COLOR_ATTACHMENT0);
    gl->glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_TRUE);

    QOpenGLShaderProgram &program = ssaoBlurProgram->program;

    if (program.bind())
    {
        // SSAO map
        gl->glActiveTexture(GL_TEXTURE0);
        gl->glBindTexture(GL_TEXTURE_2D, rt5);
        program.setUniformValue("ssaoMap", 0);

        resourceManager->quad->submeshes[0]->draw();

        program.release();
    }

    gl->glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
}

void DeferredRenderer::passLights(Camera *camera)
{
    OpenGLErrorGuard guard("DeferredRenderer::passLights()");

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

        // Render ambient light
        program.setUniformValue("lightQuad", 1);
        program.setUniformValue("lightType", 2);
        program.setUniformValue("irradianceMap", 4);
        program.setUniformValue("environmentMap", 5);
        if (g_Environment != nullptr) {
            g_Environment->irradianceMap->bind(4);
            g_Environment->environmentMap->bind(5);
        } else {
            resourceManager->texCubeDefaultIrradiance->bind(4);
            resourceManager->texCubeDefaultIrradiance->bind(5);
        }
        resourceManager->quad->submeshes[0]->draw();

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
    OpenGLErrorGuard guard("DeferredRenderer::passBackground()");

    GLenum drawBuffers[] = { GL_COLOR_ATTACHMENT3 };
    gl->glDrawBuffers(1, drawBuffers);

    OpenGLState glState;
    glState.depthTest = true;
    glState.depthFunc = GL_LEQUAL;
    glState.apply();

    QOpenGLShaderProgram &program = backgroundProgram->program;

    if (program.bind())
    {
        // Camera parameters
        QVector4D viewportParams = camera->getLeftRightBottomTop();
        program.setUniformValue("viewportSize", QVector2D(viewportWidth, viewportHeight));
        program.setUniformValue("left", viewportParams.x());
        program.setUniformValue("right", viewportParams.y());
        program.setUniformValue("bottom", viewportParams.z());
        program.setUniformValue("top", viewportParams.w());
        program.setUniformValue("znear", camera->znear);
        program.setUniformValue("worldMatrix", camera->worldMatrix);

        // Background color
        program.setUniformValue("backgroundColor", scene->backgroundColor);

        // Background texture
        auto environment = (Environment*)scene->findComponent(ComponentType::Environment);
        if (environment != nullptr && environment->texture != nullptr && environment->needsProcessing == false)
        {
            program.setUniformValue("environmentMap", 0);
            program.setUniformValue("useEnvironmentMap", true);
            environment->environmentMap->bind(0);
            program.setUniformValue("irradianceMap", 1);
            program.setUniformValue("useIrradianceMap", true);
            environment->irradianceMap->bind(1);
        }
        else
        {
            program.setUniformValue("useEnvironmentMap", false);
            program.setUniformValue("useIrradianceMap", false);
        }

        resourceManager->quad->submeshes[0]->draw();

        program.release();
    }
}

void DeferredRenderer::passSelectionOutline(Camera *camera)
{
    if (scene->renderSelectionOutline == false) return;
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

                if (entity->meshRenderer != nullptr)
                {
                    auto mesh = entity->meshRenderer->mesh;

                    if (mesh != nullptr)
                    {
                        program.setUniformValue("worldMatrix", entity->transform->matrix());

                        for (auto submesh : mesh->submeshes)
                        {
                            submesh->draw();
                        }
                    }
                }

                if (entity->lightSource != nullptr)
                {
                    QMatrix4x4 worldMatrix = entity->transform->matrix();
                    QMatrix3x3 normalMatrix = worldMatrix.normalMatrix();
                    worldMatrix.scale(0.1f, 0.1f, 0.1f);
                    program.setUniformValue("worldMatrix", worldMatrix);
                    resourceManager->sphere->submeshes[0]->draw();
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
    if (scene->renderGrid == false) return;

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

void DeferredRenderer::passMotionBlur(Camera *camera)
{
    //if (scene->renderMotionBlur == false) return;

    GLenum drawBuffers[] = { GL_COLOR_ATTACHMENT5 };
    gl->glDrawBuffers(1, drawBuffers);

    OpenGLState glState;
    glState.depthTest = false;
    glState.apply();

    QOpenGLShaderProgram &program = blurProgram->program;

    static QMatrix4x4 viewMatrixPrev = camera->viewMatrix;

    if (program.bind())
    {
        // Camera parameters
        QMatrix4x4 viewMatrix = camera->viewMatrix;
        QMatrix4x4 viewMatrixInv = viewMatrix.inverted();
        QVector4D viewportParams = camera->getLeftRightBottomTop();
        program.setUniformValue("viewportSize", QVector2D(viewportWidth, viewportHeight));
        program.setUniformValue("viewMatrixInv", viewMatrixInv);
        program.setUniformValue("left", viewportParams.x());
        program.setUniformValue("right", viewportParams.y());
        program.setUniformValue("bottom", viewportParams.z());
        program.setUniformValue("top", viewportParams.w());
        program.setUniformValue("znear", camera->znear);
        program.setUniformValue("zfar", camera->zfar);

        gl->glActiveTexture(GL_TEXTURE0);
        gl->glBindTexture(GL_TEXTURE_2D, rt3);
        program.setUniformValue("colorMap", 0);

        gl->glActiveTexture(GL_TEXTURE1);
        gl->glBindTexture(GL_TEXTURE_2D, rt4);
        program.setUniformValue("depthMap", 1);

        QMatrix4x4 viewProjectionMatrixInv = (camera->projectionMatrix * camera->viewMatrix).inverted();
        program.setUniformValue("viewProjectionMatrixInv", viewProjectionMatrixInv);

        QMatrix4x4 viewProjectionMatrixPrev = camera->projectionMatrix * viewMatrixPrev;
        program.setUniformValue("viewProjectionMatrixPrev", viewProjectionMatrixPrev);

        resourceManager->quad->submeshes[0]->draw();

        program.release();
    }

    viewMatrixPrev = camera->viewMatrix;
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
