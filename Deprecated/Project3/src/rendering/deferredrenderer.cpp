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
#include <random>

#define TEXNAME_FINAL "Final render"
#define TEXNAME_ALBED "Albedo"
#define TEXNAME_OCCLU "Occlusion"
#define TEXNAME_SPECU "Specular"
#define TEXNAME_ROUGH "Roughness"
#define TEXNAME_NORML "Normals"
#define TEXNAME_DEPTH "Depth"
#define TEXNAME_TEMP1 "Temp 1"
#define TEXNAME_WATER_REFLECTION "Water reflection"
#define TEXNAME_WATER_REFRACTION "Water refraction"
#define TEXNAME_BRIGHTEST_PIXELS "Brightest pixels"
#define TEXNAME_BLUR_HORIZONTAL "Blur horizontal"

#define WATER_TEX_DIVISOR 2

DeferredRenderer::DeferredRenderer()
{
    fbo = nullptr;
    fboReflection = nullptr;
    fboRefraction = nullptr;
    fboMousePicking = nullptr;

    // List of textures
    addTexture(TEXNAME_FINAL);
    addTexture(TEXNAME_ALBED);
    addTexture(TEXNAME_OCCLU);
    addTexture(TEXNAME_SPECU);
    addTexture(TEXNAME_ROUGH);
    addTexture(TEXNAME_NORML);
    addTexture(TEXNAME_DEPTH);
    addTexture(TEXNAME_TEMP1);
    addTexture(TEXNAME_WATER_REFLECTION);
    addTexture(TEXNAME_WATER_REFRACTION);
    addTexture(TEXNAME_BRIGHTEST_PIXELS);
    addTexture(TEXNAME_BLUR_HORIZONTAL);
}

DeferredRenderer::~DeferredRenderer()
{
    delete fbo;
    delete fboReflection;
    delete fboRefraction;
}

void DeferredRenderer::initialize()
{
    // Initialize queries

    gl->glGenQueries(sizeof(queries)/sizeof(queries[0]), queries);
    queriesFront = queries;
    queriesBack = queries + QUERY_COUNT;
    gl->glBeginQuery(GL_TIME_ELAPSED, queriesFront[QUERY_RENDER]);

    OpenGLErrorGuard guard("DeferredRenderer::initialize()");

    // Initialize GL debug

    unsigned char *font_pixels = (unsigned char*)malloc(512*512);
    for (int i = 0; i < 512*512; ++i) {
        font_pixels[i] = i %256;
    }
    Q_ASSERT(font_pixels != nullptr);
    GLDebugInit(&glDebug, font_pixels, 512, 512);
    gl->glGenTextures(1, &fontTexture);
    gl->glBindTexture(GL_TEXTURE_2D, fontTexture);
    gl->glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, 512, 512, 0, GL_RED, GL_UNSIGNED_BYTE, font_pixels);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    free(font_pixels);

    // Initialize depth shadow maps

    gl->glGenTextures(1, &shadowMap);
    gl->glBindTexture(GL_TEXTURE_2D, shadowMap);
    gl->glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, shadowMapWidth, shadowMapHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    fboShadowmap = new FramebufferObject;
    fboShadowmap->create();
    fboShadowmap->bind();
    fbo->addDepthAttachment(shadowMap);
    fbo->checkStatus();
    fbo->release();

    // Create programs

    environmentToCubemapProgram = resourceManager->createShaderProgram();
    environmentToCubemapProgram->name = "Environment to cubemap";
    environmentToCubemapProgram->vertexShaderFilename = "res/shaders/environment_to_cubemap.vert";
    environmentToCubemapProgram->fragmentShaderFilename = "res/shaders/environment_to_cubemap.frag";
    environmentToCubemapProgram->includeForSerialization = false;

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

    shadowmapProgram = resourceManager->createShaderProgram();
    shadowmapProgram->name = "Shadow map";
    shadowmapProgram->vertexShaderFilename = "res/shaders/shadowmap.vert";
    shadowmapProgram->fragmentShaderFilename = "res/shaders/shadowmap.frag";
    shadowmapProgram->includeForSerialization = false;

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

    motionBlurProgram = resourceManager->createShaderProgram();
    motionBlurProgram->name = "Blur";
    motionBlurProgram->vertexShaderFilename = "res/shaders/motion_blur.vert";
    motionBlurProgram->fragmentShaderFilename = "res/shaders/motion_blur.frag";
    motionBlurProgram->includeForSerialization = false;

    blitProgram = resourceManager->createShaderProgram();
    blitProgram->name = "Blit";
    blitProgram->vertexShaderFilename = "res/shaders/blit.vert";
    blitProgram->fragmentShaderFilename = "res/shaders/blit.frag";
    blitProgram->includeForSerialization = false;

    blitCubeProgram = resourceManager->createShaderProgram();
    blitCubeProgram->name = "Blit cube";
    blitCubeProgram->vertexShaderFilename = "res/shaders/blit_cube.vert";
    blitCubeProgram->fragmentShaderFilename = "res/shaders/blit_cube.frag";
    blitCubeProgram->includeForSerialization = false;

    forwardWithClippingProgram = resourceManager->createShaderProgram();
    forwardWithClippingProgram->name = "Forward with clipping";
    forwardWithClippingProgram->vertexShaderFilename = "res/shaders/forward_shading_with_clipping.vert";
    forwardWithClippingProgram->fragmentShaderFilename = "res/shaders/forward_shading_with_clipping.frag";
    forwardWithClippingProgram->includeForSerialization = false;

    waterProgram = resourceManager->createShaderProgram();
    waterProgram->name = "Water";
    waterProgram->vertexShaderFilename = "res/shaders/water.vert";
    waterProgram->fragmentShaderFilename = "res/shaders/water.frag";
    waterProgram->includeForSerialization = false;

    blitBrightestPixelsProgram = resourceManager->createShaderProgram();
    blitBrightestPixelsProgram->name = "Blit brightest pixels";
    blitBrightestPixelsProgram->vertexShaderFilename = "res/shaders/blit.vert";
    blitBrightestPixelsProgram->fragmentShaderFilename = "res/shaders/blit_brightest_pixels.frag";
    blitBrightestPixelsProgram->includeForSerialization = false;

    blur = resourceManager->createShaderProgram();
    blur->name = "Blur";
    blur->vertexShaderFilename = "res/shaders/blur.vert";
    blur->fragmentShaderFilename = "res/shaders/blur.frag";
    blur->includeForSerialization = false;

    bloomProgram = resourceManager->createShaderProgram();
    bloomProgram->name = "Bloom";
    bloomProgram->vertexShaderFilename = "res/shaders/blit.vert";
    bloomProgram->fragmentShaderFilename = "res/shaders/bloom.frag";
    bloomProgram->includeForSerialization = false;

    debugProgram = resourceManager->createShaderProgram();
    debugProgram->name = "Debug geometry";
    debugProgram->vertexShaderFilename = "res/shaders/debug.vert";
    debugProgram->fragmentShaderFilename = "res/shaders/debug.frag";
    debugProgram->includeForSerialization = false;

    debugTextProgram = resourceManager->createShaderProgram();
    debugTextProgram->name = "Debug text";
    debugTextProgram->vertexShaderFilename = "res/shaders/debug_text.vert";
    debugTextProgram->fragmentShaderFilename = "res/shaders/debug_text.frag";
    debugTextProgram->includeForSerialization = false;

    mousePickingProgram = resourceManager->createShaderProgram();
    mousePickingProgram->name = "Mouse picking";
    mousePickingProgram->vertexShaderFilename = "res/shaders/mouse_picking.vert";
    mousePickingProgram->fragmentShaderFilename = "res/shaders/mouse_picking.frag";
    mousePickingProgram->includeForSerialization = false;



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
                randomFloats(generator)
            );
            sample.normalize();
            sample *= randomFloats(generator);
            float scale = (float)i / 64.0;
            scale = 0.1f + 0.9f * scale * scale; // lerp(0.1, 1.0, scale*scale)
            sample *= scale;
        } while (sample.length() > 1.0 ||
                 QVector3D::dotProduct(QVector3D(0, 0, 1), sample.normalized()) < 0.1f);
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

    fboBloom1 = new FramebufferObject;
    fboBloom1->create();

    fboBloom2 = new FramebufferObject;
    fboBloom2->create();

    fboBloom3 = new FramebufferObject;
    fboBloom3->create();

    fboBloom4 = new FramebufferObject;
    fboBloom4->create();

    fboBloom5 = new FramebufferObject;
    fboBloom5->create();

    fboReflection = new FramebufferObject;
    fboReflection->create();

    fboRefraction = new FramebufferObject;
    fboRefraction->create();

    fboMousePicking = new FramebufferObject;
    fboMousePicking->create();


    // Environment map
    environmentMapResolution = 512;
    gl->glGenTextures(1, &environmentMap);
    gl->glBindTexture(GL_TEXTURE_CUBE_MAP, environmentMap);
    for (unsigned int i = 0; i < 6; ++i) {
        // note that we store each face with 16 bit floating point values
        gl->glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                     0, GL_RGB16F,
                     environmentMapResolution, environmentMapResolution, 0,
                     GL_RGB, GL_FLOAT, nullptr);
    }
    gl->glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    gl->glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    gl->glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    gl->glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    gl->glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    gl->glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

    // Irradiance map
    irradianceMapResolution = 32;
    gl->glGenTextures(1, &irradianceMap);
    gl->glBindTexture(GL_TEXTURE_CUBE_MAP, irradianceMap);
    for (unsigned int i = 0; i < 6; ++i) {
        // note that we store each face with 16 bit floating point values
        gl->glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                     0, GL_RGB16F,
                     irradianceMapResolution, irradianceMapResolution, 0,
                     GL_RGB, GL_FLOAT, nullptr);
    }
    gl->glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    gl->glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    gl->glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    gl->glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    gl->glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);


    gl->glEndQuery(GL_TIME_ELAPSED);
}

void DeferredRenderer::finalize()
{
    gl->glDeleteQueries(sizeof(queries)/sizeof(queries[0]), queries);

    gl->glDeleteTextures(1, &rtMousePicking);
    fboMousePicking->destroy();

    gl->glDeleteTextures(1, &environmentMap);
    gl->glDeleteTextures(1, &irradianceMap);

    fbo->destroy();
    fboBloom1->destroy();
    fboBloom2->destroy();
    fboBloom3->destroy();
    fboBloom4->destroy();
    fboBloom5->destroy();
    fboShadowmap->destroy();
    delete fbo;
    delete fboBloom1;
    delete fboBloom2;
    delete fboBloom3;
    delete fboBloom4;
    delete fboBloom5;
    delete fboShadowmap;

    fboReflection->destroy();
    delete fboReflection;

    fboRefraction->destroy();
    delete fboRefraction;

    GLuint textures[] = { rt0, rt1, rt2, rt3, rt5, rtD, rtReflection, rtRefraction, rtReflectionDepth, rtRefractionDepth, ssaoNoiseTex, rtBright, rtBloomH };
    gl->glDeleteTextures(13, textures);

    gl->glDeleteBuffers(1, &instancingVBO);
    for (auto &instanceGroup : instanceGroupArray)
    {
        gl->glDeleteVertexArrays(1, &instanceGroup.vao);
    }

    gl->glDeleteTextures(1, &shadowMap);

    gl->glDeleteTextures(1, &fontTexture);
    GLDebugCleanup(&glDebug);
}

void DeferredRenderer::resize(int w, int h)
{
    OpenGLErrorGuard guard("DeferredRenderer::resize()");

    // Get size
    viewportWidth = w;
    viewportHeight = h;
    glDebug.viewportWidth = w;
    glDebug.viewportHeight = h;


    // Regenerate textures

	// Albedo / occlusion
    if (rt0 != 0) gl->glDeleteTextures(1, &rt0);
    gl->glGenTextures(1, &rt0);
    gl->glBindTexture(GL_TEXTURE_2D, rt0);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    gl->glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

	// Specular / roughness
    if (rt1 != 0) gl->glDeleteTextures(1, &rt1);
    gl->glGenTextures(1, &rt1);
    gl->glBindTexture(GL_TEXTURE_2D, rt1);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    gl->glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

	// World normal / unused
    if (rt2 != 0) gl->glDeleteTextures(1, &rt2);
    gl->glGenTextures(1, &rt2);
    gl->glBindTexture(GL_TEXTURE_2D, rt2);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    gl->glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, w, h, 0, GL_RGBA, GL_FLOAT, nullptr);

    // Light emission
    if (rt3 != 0) gl->glDeleteTextures(1, &rt3);
    gl->glGenTextures(1, &rt3);
    gl->glBindTexture(GL_TEXTURE_2D, rt3);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    gl->glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, w, h, 0, GL_RGBA, GL_FLOAT, nullptr);

	// Depth texture
    if (rtD != 0) gl->glDeleteTextures(1, &rtD);
    gl->glGenTextures(1, &rtD);
    gl->glBindTexture(GL_TEXTURE_2D, rtD);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    gl->glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, w, h, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);

    if (rt5 != 0) gl->glDeleteTextures(1, &rt5);
    gl->glGenTextures(1, &rt5);
    gl->glBindTexture(GL_TEXTURE_2D, rt5);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    gl->glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, w, h, 0, GL_RGBA, GL_FLOAT, nullptr);

    if (rtReflection != 0) gl->glDeleteTextures(1, &rtReflection);
    gl->glGenTextures(1, &rtReflection);
    gl->glBindTexture(GL_TEXTURE_2D, rtReflection);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    gl->glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w/WATER_TEX_DIVISOR, h/WATER_TEX_DIVISOR, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    if (rtRefraction != 0) gl->glDeleteTextures(1, &rtRefraction);
    gl->glGenTextures(1, &rtRefraction);
    gl->glBindTexture(GL_TEXTURE_2D, rtRefraction);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    gl->glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w/WATER_TEX_DIVISOR, h/WATER_TEX_DIVISOR, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    if (rtRefractionDepth != 0) gl->glDeleteTextures(1, &rtRefractionDepth);
    gl->glGenTextures(1, &rtRefractionDepth);
    gl->glBindTexture(GL_TEXTURE_2D, rtRefractionDepth);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    gl->glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, w/WATER_TEX_DIVISOR, h/WATER_TEX_DIVISOR, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);

    if (rtReflectionDepth != 0) gl->glDeleteTextures(1, &rtReflectionDepth);
    gl->glGenTextures(1, &rtReflectionDepth);
    gl->glBindTexture(GL_TEXTURE_2D, rtReflectionDepth);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    gl->glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, w/WATER_TEX_DIVISOR, h/WATER_TEX_DIVISOR, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);

#define MIPMAP_BASE_LEVEL 0
#define MIPMAP_MAX_LEVEL 4

    // Bloom mipmap
    if (rtBright != 0) gl->glDeleteTextures(1, &rtBright);
    gl->glGenTextures(1, &rtBright);
    gl->glBindTexture(GL_TEXTURE_2D, rtBright);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, MIPMAP_BASE_LEVEL);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, MIPMAP_MAX_LEVEL);
    gl->glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, w/2, h/2, 0, GL_RGBA, GL_FLOAT, nullptr);
    gl->glTexImage2D(GL_TEXTURE_2D, 1, GL_RGBA16F, w/4, h/4, 0, GL_RGBA, GL_FLOAT, nullptr);
    gl->glTexImage2D(GL_TEXTURE_2D, 2, GL_RGBA16F, w/8, h/8, 0, GL_RGBA, GL_FLOAT, nullptr);
    gl->glTexImage2D(GL_TEXTURE_2D, 3, GL_RGBA16F, w/16, h/16, 0, GL_RGBA, GL_FLOAT, nullptr);
    gl->glTexImage2D(GL_TEXTURE_2D, 4, GL_RGBA16F, w/32, h/32, 0, GL_RGBA, GL_FLOAT, nullptr);
    gl->glGenerateMipmap(GL_TEXTURE_2D);

    // Bloom mipmap
    if (rtBloomH != 0) gl->glDeleteTextures(1, &rtBloomH);
    gl->glGenTextures(1, &rtBloomH);
    gl->glBindTexture(GL_TEXTURE_2D, rtBloomH);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, MIPMAP_BASE_LEVEL);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, MIPMAP_MAX_LEVEL);
    gl->glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, w/2, h/2, 0, GL_RGBA, GL_FLOAT, nullptr);
    gl->glTexImage2D(GL_TEXTURE_2D, 1, GL_RGBA16F, w/4, h/4, 0, GL_RGBA, GL_FLOAT, nullptr);
    gl->glTexImage2D(GL_TEXTURE_2D, 2, GL_RGBA16F, w/8, h/8, 0, GL_RGBA, GL_FLOAT, nullptr);
    gl->glTexImage2D(GL_TEXTURE_2D, 3, GL_RGBA16F, w/16, h/16, 0, GL_RGBA, GL_FLOAT, nullptr);
    gl->glTexImage2D(GL_TEXTURE_2D, 4, GL_RGBA16F, w/32, h/32, 0, GL_RGBA, GL_FLOAT, nullptr);
    gl->glGenerateMipmap(GL_TEXTURE_2D);

    // Texture for mouse picking identifiers
    if (rtMousePicking != 0) gl->glDeleteTextures(1, &rtMousePicking);
    gl->glGenTextures(1, &rtMousePicking);
    gl->glBindTexture(GL_TEXTURE_2D, rtMousePicking);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    gl->glTexImage2D(GL_TEXTURE_2D, 0, GL_R16F, w, h, 0, GL_RED, GL_FLOAT, nullptr);

    // Attach textures to the fbo

    fbo->bind();
    fbo->addColorAttachment(0, rt0);
    fbo->addColorAttachment(1, rt1);
    fbo->addColorAttachment(2, rt2);
    fbo->addColorAttachment(3, rt3);
    fbo->addColorAttachment(5, rt5);
    fbo->addDepthAttachment(rtD);
    fbo->checkStatus();
    fbo->release();

    fboReflection->bind();
    fboReflection->addColorAttachment(0, rtReflection);
    fboReflection->addDepthAttachment(rtReflectionDepth);
    fboReflection->checkStatus();
    fboReflection->release();

    fboRefraction->bind();
    fboRefraction->addColorAttachment(0, rtRefraction);
    fboRefraction->addDepthAttachment(rtRefractionDepth);
    fboRefraction->checkStatus();
    fboRefraction->release();

    fboBloom1->bind();
    fboBloom1->addColorAttachment(0, rtBright, 0);
    fboBloom1->addColorAttachment(1, rtBloomH, 0);
    fboBloom1->checkStatus();
    fboBloom1->release();

    fboBloom2->bind();
    fboBloom2->addColorAttachment(0, rtBright, 1);
    fboBloom2->addColorAttachment(1, rtBloomH, 1);
    fboBloom2->checkStatus();
    fboBloom2->release();

    fboBloom3->bind();
    fboBloom3->addColorAttachment(0, rtBright, 2);
    fboBloom3->addColorAttachment(1, rtBloomH, 2);
    fboBloom3->checkStatus();
    fboBloom3->release();

    fboBloom4->bind();
    fboBloom4->addColorAttachment(0, rtBright, 3);
    fboBloom4->addColorAttachment(1, rtBloomH, 3);
    fboBloom4->checkStatus();
    fboBloom4->release();

    fboBloom5->bind();
    fboBloom5->addColorAttachment(0, rtBright, 4);
    fboBloom5->addColorAttachment(1, rtBloomH, 4);
    fboBloom5->checkStatus();
    fboBloom5->release();

    fboMousePicking->bind();
    fboMousePicking->addColorAttachment(0, rtMousePicking);
    fboMousePicking->addDepthAttachment(rtD);
    fboMousePicking->checkStatus();
    fboMousePicking->release();
}

void DeferredRenderer::render(Camera *camera)
{
    // Determine availability of the query (no need for this as we are
    // using queries in a double buffer fashion, so queries done during
    // the previous frame will have finished at this point for sure
    //int done;
    //gl->glGetQueryObjectiv(queriesFront[QUERY_RENDER], GL_QUERY_RESULT_AVAILABLE, &done);

    // Get elapsed nanoseconds
    GLuint64 useconds;
    gl->glGetQueryObjectui64v(queriesFront[QUERY_RENDER], GL_QUERY_RESULT, &useconds);
    float mseconds = (float)useconds / 1000000.0f;

    gl->glBeginQuery(GL_TIME_ELAPSED, queriesBack[QUERY_RENDER]);

    OpenGLErrorGuard guard("DeferredRenderer::render()");

    GLDebugClear(&glDebug);
    GLDebugSetColor(&glDebug, V3(1.0, 0.0, 1.0));
    GLDebugPushLine(&glDebug, V3(0.0, 0.0, 0.0), V3(0.0, 2.0, 0.0));
    char text[256];
    sprintf(text, "Frame time: %f millis.", mseconds);
    GLDebugPushString(&glDebug, text);
    sprintf(text, "FPS: %f Hz", 1000.0/mseconds);
    GLDebugPushString(&glDebug, text);

    if (scene->renderListChanged)
    {
        updateRenderListIntoGPU();
        scene->renderListChanged = false;
    }

    if (scene->environmentChanged)
    {
        generateEnvironments();
        scene->environmentChanged = false;
    }

    if (scene->renderWater)
    {
        gl->glViewport(0, 0, viewportWidth/WATER_TEX_DIVISOR, viewportHeight/WATER_TEX_DIVISOR);

        fboReflection->bind();
        Camera reflectionCamera = *camera;
        reflectionCamera.position.setY(-reflectionCamera.position.y());
        reflectionCamera.pitch = -reflectionCamera.pitch;
        reflectionCamera.viewportWidth = viewportWidth/WATER_TEX_DIVISOR;
        reflectionCamera.viewportHeight = viewportHeight/WATER_TEX_DIVISOR;
        reflectionCamera.prepareMatrices();
        passWaterScene(&reflectionCamera, GL_COLOR_ATTACHMENT0, WaterScenePart::Reflection);
        passBackground(&reflectionCamera, GL_COLOR_ATTACHMENT0);
        fboReflection->release();

        fboRefraction->bind();
        Camera refractionCamera = *camera;
        refractionCamera.position.setY(-refractionCamera.position.y());
        refractionCamera.pitch = -refractionCamera.pitch;
        refractionCamera.viewportWidth = viewportWidth/WATER_TEX_DIVISOR;
        refractionCamera.viewportHeight = viewportHeight/WATER_TEX_DIVISOR;
        refractionCamera.prepareMatrices();
        passWaterScene(camera, GL_COLOR_ATTACHMENT0, WaterScenePart::Refraction);
        fboRefraction->release();

        gl->glViewport(0, 0, viewportWidth, viewportHeight);
    }

    fbo->bind();

    clearBuffers();

    passMeshes(camera);

    if (scene->renderSSAO)
    {
        passSSAO(camera);
        passSSAOBlur();
    }

    //passShadowmaps(camera);

    passLights(camera);

    passBackground(camera, GL_COLOR_ATTACHMENT3);

    if (scene->renderWater)
    {
        passWaterPlane(camera, GL_COLOR_ATTACHMENT3);
    }

    if (scene->renderSelectionOutline && selection->count > 0)
    {
        passSelectionMask(camera, GL_COLOR_ATTACHMENT5);
        passSelectionOutline(camera, GL_COLOR_ATTACHMENT3);
    }

    if (scene->renderGrid)
    {
        passGrid(camera, GL_COLOR_ATTACHMENT3);
    }

    fbo->release();

    if (scene->renderBloom)
    {
#define LOD(x) x
        const QVector2D horizontal(1.0, 0.0);
        const QVector2D vertical(0.0, 1.0);

        const float w = viewportWidth;
        const float h = viewportHeight;

        // Mipmap of brightest intensities
        float threshold = 1.0;
        passBlitBrightPixels(fboBloom1, QVector2D(w/2, h/2), GL_COLOR_ATTACHMENT0, rt3, LOD(0), threshold);
        gl->glBindTexture(GL_TEXTURE_2D, rtBright);
        gl->glGenerateMipmap(GL_TEXTURE_2D);

        //       Dest FBO   Dest resolution        Dest attachment       Src tex   Src LOD  Direction   Blur radius
        passBlur(fboBloom1, QVector2D(w/2, h/2),   GL_COLOR_ATTACHMENT1, rtBright, LOD(0),  horizontal, scene->bloomRadius);
        passBlur(fboBloom2, QVector2D(w/4, h/4),   GL_COLOR_ATTACHMENT1, rtBright, LOD(1),  horizontal, scene->bloomRadius);
        passBlur(fboBloom3, QVector2D(w/8, h/8),   GL_COLOR_ATTACHMENT1, rtBright, LOD(2),  horizontal, scene->bloomRadius);
        passBlur(fboBloom4, QVector2D(w/16, h/16), GL_COLOR_ATTACHMENT1, rtBright, LOD(3),  horizontal, scene->bloomRadius);
        passBlur(fboBloom5, QVector2D(w/32, h/32), GL_COLOR_ATTACHMENT1, rtBright, LOD(4),  horizontal, scene->bloomRadius);

        passBlur(fboBloom1, QVector2D(w/2, h/2),   GL_COLOR_ATTACHMENT0, rtBloomH, LOD(0),  vertical,   scene->bloomRadius);
        passBlur(fboBloom2, QVector2D(w/4, h/4),   GL_COLOR_ATTACHMENT0, rtBloomH, LOD(1),  vertical,   scene->bloomRadius);
        passBlur(fboBloom3, QVector2D(w/8, h/8),   GL_COLOR_ATTACHMENT0, rtBloomH, LOD(2),  vertical,   scene->bloomRadius);
        passBlur(fboBloom4, QVector2D(w/16, h/16), GL_COLOR_ATTACHMENT0, rtBloomH, LOD(3),  vertical,   scene->bloomRadius);
        passBlur(fboBloom5, QVector2D(w/32, h/32), GL_COLOR_ATTACHMENT0, rtBloomH, LOD(4),  vertical,   scene->bloomRadius);

        passBloom(fbo, GL_COLOR_ATTACHMENT3, rtBright, 4);

#undef LOD
    }

//    passMotionBlur(camera);

    passBlit();

    passDebug();

    passMousePicking(camera);

    gl->glEndQuery(GL_TIME_ELAPSED);

    // Swap queries buffer
    GLuint *queriesTmp = queriesFront;
    queriesFront = queriesBack;
    queriesBack = queriesTmp;
}

void DeferredRenderer::generateEnvironments()
{
    OpenGLErrorGuard guard("DeferredRenderer::generateEnvironments()");

    // Create temporary FBO
    unsigned int captureFBO;
    gl->glGenFramebuffers(1, &captureFBO);
    gl->glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);

    // Create temporary cubemap
    unsigned int tmpCube;
    gl->glGenTextures(1, &tmpCube);
    gl->glBindTexture(GL_TEXTURE_CUBE_MAP, tmpCube);
    for (unsigned int i = 0; i < 6; ++i) {
        gl->glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                     0, GL_RGB16F,
                     irradianceMapResolution, irradianceMapResolution, 0,
                     GL_RGB, GL_FLOAT, nullptr);
    }
    gl->glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    gl->glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    gl->glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    gl->glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    gl->glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

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

    OpenGLState glState;
    glState.faceCulling = false;
    glState.apply();

    QOpenGLShaderProgram *program = nullptr;


    // convert HDR equirectangular environment map to cubemap equivalent
    program = &environmentToCubemapProgram->program;
    if (program->bind())
    {
        program->setUniformValue("projectionMatrix", captureProjection);

        bool environmentImageAvailable = false;
        GLuint equirectangularTexture = resourceManager->texWhite->textureId();
        // NOTE(jesus): Chapucilla, only processing one single environment...
        auto environment = Environment::instance;
        if (environment != nullptr && environment->texture != nullptr) {
            environmentImageAvailable = true;
            equirectangularTexture = environment->texture->textureId();
        }

        gl->glActiveTexture(GL_TEXTURE0);
        gl->glBindTexture(GL_TEXTURE_2D, equirectangularTexture);
        program->setUniformValue("equirectangularMap", 0);
        program->setUniformValue("equirectangularMapAvailable", environmentImageAvailable);
        program->setUniformValue("backgroundColor", scene->backgroundColor);

        gl->glViewport(0, 0, environmentMapResolution, environmentMapResolution);
        gl->glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);

        for (unsigned int i = 0; i < 6; ++i)
        {
            program->setUniformValue("viewMatrix", captureViews[i]);
            gl->glFramebufferTexture2D(GL_FRAMEBUFFER,
                                       GL_COLOR_ATTACHMENT0,
                                       GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                                       environmentMap,
                                       0);
            resourceManager->cube->submeshes[0]->draw();
        }

        program->release();
    }

    gl->glBindTexture(GL_TEXTURE_CUBE_MAP, environmentMap);
    gl->glGenerateMipmap(GL_TEXTURE_CUBE_MAP);


    // blit High resolution cubemap into low-res cubemap
    program = &blitCubeProgram->program;
    if (program->bind())
    {
        program->setUniformValue("cubeMap", 0);
        program->setUniformValue("projectionMatrix", captureProjection);

        gl->glActiveTexture(GL_TEXTURE0);
        gl->glBindTexture(GL_TEXTURE_CUBE_MAP, environmentMap);

        gl->glViewport(0, 0, irradianceMapResolution, irradianceMapResolution);
        gl->glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);

        for (unsigned int i = 0; i < 6; ++i)
        {
            program->setUniformValue("viewMatrix", captureViews[i]);
            gl->glFramebufferTexture2D(GL_FRAMEBUFFER,
                                       GL_COLOR_ATTACHMENT0,
                                       GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                                       tmpCube,
                                       0);
            resourceManager->cube->submeshes[0]->draw();
        }

        program->release();
    }

    // create HDR irradiance cubemap from low res cubemap
    program = &irradianceProgram->program;
    if (program->bind())
    {
        program->setUniformValue("environmentMap", 0);
        program->setUniformValue("projectionMatrix", captureProjection);

        gl->glActiveTexture(GL_TEXTURE0);
        gl->glBindTexture(GL_TEXTURE_CUBE_MAP, tmpCube);

        gl->glViewport(0, 0, irradianceMapResolution, irradianceMapResolution);
        gl->glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);

        for (unsigned int i = 0; i < 6; ++i)
        {
            program->setUniformValue("viewMatrix", captureViews[i]);
            gl->glFramebufferTexture2D(GL_FRAMEBUFFER,
                                       GL_COLOR_ATTACHMENT0,
                                       GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                                       irradianceMap,
                                       0);
            resourceManager->cube->submeshes[0]->draw();
        }

        program->release();
    }

    gl->glDeleteTextures(1, &tmpCube);
    gl->glDeleteFramebuffers(1, &captureFBO);

    gl->glViewport(0, 0, viewportWidth, viewportHeight);
}

extern int g_MaxSubmeshes;

void DeferredRenderer::clearBuffers()
{
#if 0
    GLuint rt0 = 0; // Albedo (RGB), occlussion (A)
    GLuint rt1 = 0; // Specular (RGB), roughness (A)
    GLuint rt2 = 0; // World normal (RGB), unused (A)
    GLuint rt3 = 0; // Light (Emission + lightmaps + lighting pass) (RGB), unused (A)
    GLuint rt5 = 0; // Tmp RGBA
    GLuint rtD = 0; // Depth + stencil
#endif
    float albedo_occlusion[] = {0.0f, 0.0f, 0.0f, 1.0f};
    float specular_roughness[] = {0.0f, 0.0f, 0.0f, 1.0f};
    float normal_unused[] = {0.0f, 0.0f, 0.0f, 1.0f};
    float light_unused[] = {0.0f, 0.0f, 0.0f, 1.0f};
    float depth_value = 1.0;
    gl->glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    gl->glClearBufferfv(GL_COLOR, 0, albedo_occlusion);
    gl->glClearBufferfv(GL_COLOR, 1, specular_roughness);
    //gl->glClearBufferfv(GL_COLOR, 2, normal_unused);
    gl->glClearBufferfv(GL_COLOR, 3, light_unused);
    gl->glClearBufferfv(GL_DEPTH, 0, &depth_value);
}

void DeferredRenderer::passWaterScene(Camera *camera, GLenum colorAttachment, WaterScenePart part)
{
    OpenGLErrorGuard guard("DeferredRenderer::passWaterScene()");

    gl->glDrawBuffer(colorAttachment);

    OpenGLState glState;
    glState.depthTest = true;
    glState.clipDistance[0] = true;
    glState.apply();

    gl->glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    gl->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    QOpenGLShaderProgram &program = forwardWithClippingProgram->program;

    if (program.bind())
    {
        QMatrix4x4 viewMatrix = camera->viewMatrix;
        program.setUniformValue("viewMatrix", viewMatrix);
        program.setUniformValue("projectionMatrix", camera->projectionMatrix);
        program.setUniformValue("eyeWorldspace", QVector3D(camera->worldMatrix * QVector4D(0.0, 0.0, 0.0, 1.0)));

        if (part == WaterScenePart::Reflection)
        {
            program.setUniformValue("clippingPlane", QVector4D(0,1,0,0));
        }
        else
        {
            program.setUniformValue("clippingPlane", QVector4D(0,-1,0,0));
        }


        Material *material = nullptr;

#define SEND_TEXTURE(uniformName, tex1, tex2, texUnit) \
    program.setUniformValue(uniformName, texUnit); \
    if (tex1 != nullptr) { tex1->bind(texUnit); } \
    else                 { tex2->bind(texUnit); }

        for (auto &instanceGroup : instanceGroupArray)
        {
            if (material != instanceGroup.material)
            {
                material = instanceGroup.material;

                // Switch between texture / no-texture
                float metalness = material->metalness;
                if (material->specularTexture != nullptr) {
                    metalness = 1.0f;
                }

                // Send the material to the shader
                program.setUniformValue("albedo",     material->albedo);
                program.setUniformValue("emissive",   material->emissive);
                program.setUniformValue("specular",   material->specular);
                program.setUniformValue("smoothness", material->smoothness);
                program.setUniformValue("metalness",  metalness);
                program.setUniformValue("bumpiness",  material->bumpiness);
                program.setUniformValue("tiling", material->tiling);
                SEND_TEXTURE("albedoTexture",   material->albedoTexture,   resourceManager->texWhite, 0);
                SEND_TEXTURE("emissiveTexture", material->emissiveTexture, resourceManager->texBlack, 1);
                SEND_TEXTURE("specularTexture", material->specularTexture, resourceManager->texWhite, 2);
                SEND_TEXTURE("normalTexture",   material->normalsTexture,  resourceManager->texNormal, 3);
                SEND_TEXTURE("bumpTexture",     material->bumpTexture,     resourceManager->texWhite, 4);
                program.setUniformValue("irradianceMap", 5);
                gl->glActiveTexture(GL_TEXTURE5);
                gl->glBindTexture(GL_TEXTURE_CUBE_MAP, irradianceMap);
//                if (Environment::instance != nullptr) {
//                    Environment::instance->irradianceMap->bind(5);
//                } else {
//                    resourceManager->texCubeDefaultIrradiance->bind(5);
//                }
            }

            // Render geometry
            gl->glBindVertexArray(instanceGroup.vao);
            instanceGroup.submesh->drawInstanced(instanceGroup.count);
            gl->glBindVertexArray(0);
        }

#undef SEND_TEXTURE

        program.release();
    }
}

void DeferredRenderer::passWaterPlane(Camera *camera, GLenum colorAttachment)
{
    OpenGLErrorGuard guard("DeferredRenderer::passWaterPlane()");

    gl->glDrawBuffer(colorAttachment);

    OpenGLState glState;
    glState.depthTest = true;
    glState.depthWrite = false;
    glState.blending = false;
    //glState.blendFuncDst = GL_ONE_MINUS_SRC_ALPHA;
    //glState.blendFuncSrc = GL_SRC_ALPHA;
    glState.apply();

    QOpenGLShaderProgram &program = waterProgram->program;

    if (program.bind())
    {
        for (auto entity : scene->entities)
        {
            auto meshRenderer = entity->meshRenderer;
            if (meshRenderer == nullptr) continue;
            if (meshRenderer->materials.empty()) continue;
            auto material = meshRenderer->materials[0];
            if (material == nullptr) continue;
            if (material->shaderType == MaterialShaderType::Water)
            {
                // Camera parameters
                QMatrix4x4 projectionMatrix = camera->projectionMatrix;
                QMatrix4x4 viewMatrix = camera->viewMatrix;
                QMatrix4x4 worldMatrix = entity->transform->matrix();
                QMatrix4x4 projectionMatrixInv = projectionMatrix.inverted();
                QMatrix4x4 viewMatrixInv = viewMatrix.inverted();

                program.setUniformValue("viewportSize", QVector2D(camera->viewportWidth, camera->viewportHeight));
                program.setUniformValue("projectionMatrix", projectionMatrix);
                program.setUniformValue("viewMatrix", viewMatrix);
                program.setUniformValue("worldViewMatrix", viewMatrix * worldMatrix);
                program.setUniformValue("projectionMatrixInv", projectionMatrixInv);
                program.setUniformValue("viewMatrixInv", viewMatrixInv);

                gl->glActiveTexture(GL_TEXTURE0);
                gl->glBindTexture(GL_TEXTURE_2D, rtReflection);
                program.setUniformValue("reflectionMap", 0);

                gl->glActiveTexture(GL_TEXTURE1);
                gl->glBindTexture(GL_TEXTURE_2D, rtRefraction);
                program.setUniformValue("refractionMap", 1);

                gl->glActiveTexture(GL_TEXTURE2);
                gl->glBindTexture(GL_TEXTURE_2D, rtReflectionDepth);
                program.setUniformValue("reflectionDepth", 2);

                gl->glActiveTexture(GL_TEXTURE3);
                gl->glBindTexture(GL_TEXTURE_2D, rtRefractionDepth);
                program.setUniformValue("refractionDepth", 3);

                gl->glActiveTexture(GL_TEXTURE4);
                gl->glBindTexture(GL_TEXTURE_2D, resourceManager->texWaterNormals->textureId());
                program.setUniformValue("normalMap", 4);

                gl->glActiveTexture(GL_TEXTURE5);
                gl->glBindTexture(GL_TEXTURE_2D, resourceManager->texWaterDudv->textureId());
                program.setUniformValue("dudvMap", 5);


                for (auto submesh : meshRenderer->mesh->submeshes)
                {
                    submesh->draw();
                }

                break;
            }
        }
        program.release();
    }
}

#if 1
// Optimized version with instancing
void DeferredRenderer::passMeshes(Camera *camera)
{
    OpenGLErrorGuard guard("DeferredRenderer::passMeshes()");

    GLenum drawBuffers[] = {
        GL_COLOR_ATTACHMENT0,
        GL_COLOR_ATTACHMENT1,
        GL_COLOR_ATTACHMENT2,
        GL_COLOR_ATTACHMENT3
    };
    gl->glDrawBuffers(4, drawBuffers);

    gl->glClear(GL_DEPTH_BUFFER_BIT);

    OpenGLState glState;
    glState.depthTest = true;
    glState.apply();

    QOpenGLShaderProgram &program = materialProgram->program;

    if (program.bind())
    {
        QMatrix4x4 viewMatrix = camera->viewMatrix;
        program.setUniformValue("viewMatrix", viewMatrix);
        program.setUniformValue("projectionMatrix", camera->projectionMatrix);
        program.setUniformValue("eyeWorldspace", QVector3D(camera->worldMatrix * QVector4D(0.0, 0.0, 0.0, 1.0)));

        Material *material = nullptr;

#define SEND_TEXTURE(uniformName, tex1, tex2, texUnit) \
    program.setUniformValue(uniformName, texUnit); \
    if (tex1 != nullptr) { tex1->bind(texUnit); } \
    else                 { tex2->bind(texUnit); }

        for (auto &instanceGroup : instanceGroupArray)
        {
            if (material != instanceGroup.material)
            {
                material = instanceGroup.material;

                // Switch between texture / no-texture
                float metalness = material->metalness;
                if (material->specularTexture != nullptr) {
                    metalness = 1.0f;
                }

                // Send the material to the shader
                program.setUniformValue("albedo",     material->albedo);
                program.setUniformValue("emissive",   material->emissive);
                program.setUniformValue("specular",   material->specular);
                program.setUniformValue("smoothness", material->smoothness);
                program.setUniformValue("metalness",  metalness);
                program.setUniformValue("bumpiness",  material->bumpiness);
                program.setUniformValue("tiling", material->tiling);
                SEND_TEXTURE("albedoTexture",   material->albedoTexture,   resourceManager->texWhite, 0);
                SEND_TEXTURE("emissiveTexture", material->emissiveTexture, resourceManager->texBlack, 1);
                SEND_TEXTURE("specularTexture", material->specularTexture, resourceManager->texWhite, 2);
                SEND_TEXTURE("normalTexture",   material->normalsTexture,  resourceManager->texNormal, 3);
                SEND_TEXTURE("bumpTexture",     material->bumpTexture,     resourceManager->texWhite, 4);
            }

            // Render geometry
            gl->glBindVertexArray(instanceGroup.vao);
            instanceGroup.submesh->drawInstanced(instanceGroup.count);
            gl->glBindVertexArray(0);
        }

#undef SEND_TEXTURE

        program.release();
    }
}
#endif

#if 0
// Slightly optimized version:
// Meshes are rendered as listed in instanceArray, which is sorted by material.
// Being meshes groupd by material allows us sending less information.
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

        Material *material = nullptr;

#define SEND_TEXTURE(uniformName, tex1, tex2, texUnit) \
    program.setUniformValue(uniformName, texUnit); \
    if (tex1 != nullptr) { tex1->bind(texUnit); } \
    else                 { tex2->bind(texUnit); }

        for (auto &instance : instanceArray)
        {
            if (material != instance.material)
            {
                material = instance.material;

                // Switch between texture / no-texture
                float metalness = material->metalness;
                if (material->specularTexture != nullptr) {
                    metalness = 1.0f;
                }

                // Send the material to the shader
                program.setUniformValue("albedo",     material->albedo);
                program.setUniformValue("emissive",   material->emissive);
                program.setUniformValue("specular",   material->specular);
                program.setUniformValue("smoothness", material->smoothness);
                program.setUniformValue("metalness",  metalness);
                program.setUniformValue("bumpiness",  material->bumpiness);
                program.setUniformValue("tiling", material->tiling);
                SEND_TEXTURE("albedoTexture",   material->albedoTexture,   resourceManager->texWhite, 0);
                SEND_TEXTURE("emissiveTexture", material->emissiveTexture, resourceManager->texBlack, 1);
                SEND_TEXTURE("specularTexture", material->specularTexture, resourceManager->texWhite, 2);
                SEND_TEXTURE("normalTexture",   material->normalsTexture,  resourceManager->texNormal, 3);
                SEND_TEXTURE("bumpTexture",     material->bumpTexture,     resourceManager->texWhite, 4);
            }

            // Send transforms to shader
            QMatrix4x4 worldMatrix = instance.transform->matrix();
            QMatrix3x3 normalMatrix = worldMatrix.normalMatrix();
            program.setUniformValue("worldMatrix", worldMatrix);
            program.setUniformValue("normalMatrix", normalMatrix);

            // Render geometry
            instance.submesh->draw();
        }

#undef SEND_TEXTURE

        program.release();
    }
}
#endif

#if 0
// Unoptimized version - mesh renderers are not traversed in any specific order
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
#endif

void DeferredRenderer::passSSAO(Camera *camera)
{
    if (scene->renderSSAO == false) return;

    OpenGLErrorGuard guard("DeferredRenderer::passSSAO()");

    gl->glDrawBuffer(GL_COLOR_ATTACHMENT5);

    OpenGLState glState;
    glState.depthTest = false;
    glState.apply();

    QOpenGLShaderProgram &program = ssaoProgram->program;

    if (program.bind())
    {
        // Viewport parameters
        program.setUniformValue("viewportSize", QVector2D(camera->viewportWidth, camera->viewportHeight));

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
        gl->glBindTexture(GL_TEXTURE_2D, rtD);
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

void DeferredRenderer::passShadowmaps(Camera *camera)
{
    // TODO(jesus): Render the shadowmaps

    OpenGLErrorGuard guard("DeferredRenderer::passShadowmaps()");

    OpenGLState glState;
    glState.depthTest = true;
    glState.depthWrite = true;
    glState.colorWrite = false;
    glState.apply();

    gl->glClear(GL_DEPTH_BUFFER_BIT);

    QOpenGLShaderProgram &program = shadowmapProgram->program;

    if (program.bind())
    {
        // TODO(jesus): find the light entity

        QMatrix4x4 viewMatrix;           // TODO(jesus): inverse of the light world matrix
        QMatrix4x4 orthogonalProjection; // TODO(jesus): taking into account the camera frustum
        program.setUniformValue("viewMatrix", viewMatrix);
        program.setUniformValue("projectionMatrix", orthogonalProjection);
        program.setUniformValue("eyeWorldspace", QVector3D(camera->worldMatrix * QVector4D(0.0, 0.0, 0.0, 1.0)));

        for (auto &instanceGroup : instanceGroupArray)
        {
            // Render geometry
            gl->glBindVertexArray(instanceGroup.vao);
            instanceGroup.submesh->drawInstanced(instanceGroup.count);
            gl->glBindVertexArray(0);
        }

        program.release();
    }
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
        program.setUniformValue("rtD", 3); gl->glActiveTexture(GL_TEXTURE0 + 3); gl->glBindTexture(GL_TEXTURE_2D, rtD);

        // Viewport parameters
        program.setUniformValue("viewportSize", QVector2D(camera->viewportWidth, camera->viewportHeight));

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
        program.setUniformValue("irradianceMap", 4);  gl->glActiveTexture(GL_TEXTURE0 + 4); gl->glBindTexture(GL_TEXTURE_CUBE_MAP, irradianceMap);
        program.setUniformValue("environmentMap", 5); gl->glActiveTexture(GL_TEXTURE0 + 5); gl->glBindTexture(GL_TEXTURE_CUBE_MAP, environmentMap);

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

            // TODO(jesus): Pass shadowmap texture and light matrices
            //program.setUniformValue("shadowmap", ???)

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

void DeferredRenderer::passBackground(Camera *camera, GLenum colorAttachment)
{
    OpenGLErrorGuard guard("DeferredRenderer::passBackground()");

    gl->glDrawBuffer(colorAttachment);

    OpenGLState glState;
    glState.depthTest = true;
    glState.depthFunc = GL_LEQUAL;
    glState.apply();

    QOpenGLShaderProgram &program = backgroundProgram->program;

    if (program.bind())
    {
        // Camera parameters
        QVector4D viewportParams = camera->getLeftRightBottomTop();
        program.setUniformValue("viewportSize", QVector2D(camera->viewportWidth, camera->viewportHeight));
        program.setUniformValue("left", viewportParams.x());
        program.setUniformValue("right", viewportParams.y());
        program.setUniformValue("bottom", viewportParams.z());
        program.setUniformValue("top", viewportParams.w());
        program.setUniformValue("znear", camera->znear);
        program.setUniformValue("worldMatrix", camera->worldMatrix);

        program.setUniformValue("environmentMap", 0);
        gl->glActiveTexture(GL_TEXTURE0);
        gl->glBindTexture(GL_TEXTURE_CUBE_MAP, environmentMap);
        program.setUniformValue("irradianceMap", 1);
        gl->glActiveTexture(GL_TEXTURE1);
        gl->glBindTexture(GL_TEXTURE_CUBE_MAP, irradianceMap);

        resourceManager->quad->submeshes[0]->draw();

        program.release();
    }
}

void DeferredRenderer::passSelectionMask(Camera *camera, GLenum colorAttachment)
{
    gl->glDrawBuffer(colorAttachment);

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

void DeferredRenderer::passSelectionOutline(Camera *camera, GLenum colorAttachment)
{
    gl->glDrawBuffer(colorAttachment);

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

void DeferredRenderer::passGrid(Camera *camera, GLenum colorAttachment)
{
    gl->glDrawBuffer(colorAttachment);

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

    QOpenGLShaderProgram &program = motionBlurProgram->program;

    static QMatrix4x4 viewMatrixPrev = camera->viewMatrix;

    if (program.bind())
    {
        // Camera parameters
        QMatrix4x4 viewMatrix = camera->viewMatrix;
        QMatrix4x4 viewMatrixInv = viewMatrix.inverted();
        QVector4D viewportParams = camera->getLeftRightBottomTop();
        program.setUniformValue("viewportSize", QVector2D(camera->viewportWidth, camera->viewportHeight));
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
        gl->glBindTexture(GL_TEXTURE_2D, rtD);
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

void DeferredRenderer::passBlitBrightPixels(FramebufferObject *fbo,
                                            const QVector2D &viewportSize,
                                            GLenum colorAttachment,
                                            GLuint inputTexture,
                                            GLint inputLod,
                                            float threshold)
{
    fbo->bind();
    gl->glDrawBuffer(colorAttachment);

    gl->glViewport(0, 0, viewportSize.x(), viewportSize.y());

    OpenGLState::reset();

    QOpenGLShaderProgram &program = blitBrightestPixelsProgram->program;

    if (program.bind())
    {
        gl->glActiveTexture(GL_TEXTURE0);
        gl->glBindTexture(GL_TEXTURE_2D, inputTexture);
        gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        program.setUniformValue("colorTexture", 0);

        resourceManager->quad->submeshes[0]->draw();

        gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        program.release();
    }
}

void DeferredRenderer::passBlur(FramebufferObject *pfbo,
                                const QVector2D &viewportSize,
                                GLenum colorAttachment,
                                GLuint inputTexture,
                                GLint inputLod,
                                const QVector2D &direction,
                                float blurRadius)
{
    pfbo->bind();
    gl->glDrawBuffer(colorAttachment);
    gl->glViewport(0, 0, viewportSize.x(), viewportSize.y());

	OpenGLState glState;
	glState.depthTest = false;
	glState.blending = false;
	glState.apply();

	QOpenGLShaderProgram &program = blur->program;

	if (program.bind())
	{
		gl->glActiveTexture(GL_TEXTURE0);
		gl->glBindTexture(GL_TEXTURE_2D, inputTexture);
		program.setUniformValue("colorMap", 0);
        program.setUniformValue("inputLod", inputLod);
        program.setUniformValue("direction", direction);
        program.setUniformValue("radius", blurRadius);

        resourceManager->quad->submeshes[0]->draw();

		program.release();
	}

    pfbo->release();
}

void DeferredRenderer::passBloom(FramebufferObject *fbo,
                                 GLenum colorAttachment,
                                 GLuint inputTexture,
                                 int maxLod)
{
    fbo->bind();
    gl->glDrawBuffer(colorAttachment);

    gl->glViewport(0, 0, viewportWidth, viewportHeight);

    OpenGLState glState;
    glState.depthTest = false;
    glState.blending = true;
    glState.blendFuncDst = GL_ONE;
    glState.blendFuncSrc = GL_ONE;
    glState.apply();

    QOpenGLShaderProgram &program = bloomProgram->program;

    if (program.bind())
    {
        gl->glActiveTexture(GL_TEXTURE0);
        gl->glBindTexture(GL_TEXTURE_2D, inputTexture);
        program.setUniformValue("colorMap", 0);
        program.setUniformValue("maxLod", maxLod);

        float lodIntensities[] = {
            scene->bloomLod0Intensity,
            scene->bloomLod1Intensity,
            scene->bloomLod2Intensity,
            scene->bloomLod3Intensity,
            scene->bloomLod4Intensity
        };
        program.setUniformValueArray("lodIntensities", lodIntensities, 5, 1);

        resourceManager->quad->submeshes[0]->draw();

        program.release();
    }

    fbo->release();
}

void DeferredRenderer::passBlit()
{
    OpenGLState::reset();

    gl->glViewport(0, 0, viewportWidth, viewportHeight);

    QOpenGLShaderProgram &program = blitProgram->program;

    if (program.bind())
    {
        GLuint texId = 0;
        bool blitAlpha = false;
        bool blitDepth = false;
        QString texname = shownTexture();
        if      (texname == TEXNAME_FINAL) { texId = rt3; }
        else if (texname == TEXNAME_ALBED) { texId = rt0; }
        else if (texname == TEXNAME_OCCLU) { texId = rt0; blitAlpha = true; }
        else if (texname == TEXNAME_SPECU) { texId = rt1; }
        else if (texname == TEXNAME_ROUGH) { texId = rt1; blitAlpha = true; }
        else if (texname == TEXNAME_NORML) { texId = rt2; }
        else if (texname == TEXNAME_DEPTH) { texId = rtD; blitDepth = true; }
        else if (texname == TEXNAME_TEMP1) { texId = rt5; }
        else if (texname == TEXNAME_WATER_REFLECTION) { texId = rtReflection; }
        else if (texname == TEXNAME_WATER_REFRACTION) { texId = rtRefraction; }
        else if (texname == TEXNAME_BLUR_HORIZONTAL) { texId = rtBloomH; }
        else if (texname == TEXNAME_BRIGHTEST_PIXELS) { texId = rtBright; }

        gl->glActiveTexture(GL_TEXTURE0);
        gl->glBindTexture(GL_TEXTURE_2D, texId);
        program.setUniformValue("colorTexture", 0);
        program.setUniformValue("blitAlpha", blitAlpha);
        program.setUniformValue("blitDepth", blitDepth);
        program.setUniformValue("lod", shownLod());

        resourceManager->quad->submeshes[0]->draw();

        program.release();
    }
}

#define MAX_DEBUG_BUFFER_SIZE (1024*1024)

void DeferredRenderer::passDebug()
{
    static GLuint vao;
    static GLuint vbo;

    static bool initialized = false;
    if (!initialized)
    {
        gl->glGenVertexArrays(1, &vao);
        gl->glGenBuffers(1, &vbo);
        gl->glBindBuffer(GL_ARRAY_BUFFER, vbo);
        gl->glBufferData(GL_ARRAY_BUFFER, MAX_DEBUG_BUFFER_SIZE, nullptr, GL_STREAM_DRAW);
        gl->glBindBuffer(GL_ARRAY_BUFFER, 0);
        initialized = true;
    }

    GLuint totalSize = 0;

    GLuint offset0 = 0;
    GLuint offset00 = offset0;
    GLuint offset01 = sizeof(Vec3);
    GLuint ncomp00 = 3;
    GLuint ncomp01 = 3;
    GLuint vsize0 = (ncomp00 + ncomp01)*sizeof(float);
    GLuint size0 = glDebug.numLines * sizeof(glDebug.lines[0]);
    totalSize += size0;

    GLuint offset1 = size0;
    GLuint offset10 = offset1;
    GLuint offset11 = offset1 + sizeof(Vec3);
    GLuint ncomp10 = 3;
    GLuint ncomp11 = 2;
    GLuint vsize1 = (ncomp10 + ncomp11)*sizeof(float);
    GLuint size1 = glDebug.numQuads * sizeof(glDebug.quads[0]);
    totalSize += size1;

    Q_ASSERT(totalSize <= MAX_DEBUG_BUFFER_SIZE);

    gl->glBindBuffer(GL_ARRAY_BUFFER, vbo);
    gl->glBufferSubData(GL_ARRAY_BUFFER, offset0, size0, glDebug.lines);
    gl->glBufferSubData(GL_ARRAY_BUFFER, offset1, size1, glDebug.quads);

    gl->glBindVertexArray(vao);

    OpenGLState::reset();

    gl->glViewport(0, 0, glDebug.viewportWidth, glDebug.viewportHeight);

    if (glDebug.numLines > 0)
    {
        gl->glEnableVertexAttribArray(0);
        gl->glVertexAttribPointer(0, ncomp00, GL_FLOAT, GL_FALSE, vsize0, (void *)offset00);
        gl->glEnableVertexAttribArray(1);
        gl->glVertexAttribPointer(1, ncomp01, GL_FLOAT, GL_FALSE, vsize0, (void *)offset01);

        QOpenGLShaderProgram &program = debugProgram->program;
        if (program.bind())
        {
            program.setUniformValue("viewMatrix", camera->viewMatrix);
            program.setUniformValue("projectionMatrix", camera->projectionMatrix);
            gl->glDrawArrays(GL_LINES, 0, glDebug.numLines * 2);
            program.release();
        }
    }

    if (glDebug.numQuads > 0)
    {
        OpenGLState glstate;
        glstate.blending = true;
        glstate.blendFuncSrc = GL_SRC_ALPHA;
        glstate.blendFuncDst = GL_ONE_MINUS_SRC_ALPHA;
        glstate.depthTest = false;
        glstate.apply();
        gl->glEnableVertexAttribArray(0);
        gl->glVertexAttribPointer(0, ncomp10, GL_FLOAT, GL_FALSE, vsize1, (void *)offset10);
        gl->glEnableVertexAttribArray(1);
        gl->glVertexAttribPointer(1, ncomp11, GL_FLOAT, GL_FALSE, vsize1, (void *)offset11);

        QOpenGLShaderProgram &program = debugTextProgram->program;
        if (program.bind())
        {
            QMatrix4x4 projectionMatrix;
            projectionMatrix.ortho(0.0, viewportWidth, 0.0, viewportHeight, -1.0, 1.0);
            program.setUniformValue("projectionMatrix", projectionMatrix);
            program.setUniformValue("fontTexture", 0);
            gl->glActiveTexture(GL_TEXTURE0);
            gl->glBindTexture(GL_TEXTURE_2D, fontTexture);
            gl->glDrawArrays(GL_TRIANGLES, 0, glDebug.numQuads * 6);
            program.release();
        }
    }

    gl->glBindVertexArray(0);
    gl->glBindBuffer(GL_ARRAY_BUFFER, 0);
    //gl->glDeleteVertexArrays(1, &vao);
    //gl->glDeleteBuffers(1, &vbo);
}

void DeferredRenderer::updateRenderListIntoGPU()
{
    OpenGLErrorGuard guard("DeferredRenderer::updateRenderListIntoGPU()");

    instanceArray.clear();

    // Populate the instanceArray
    for (auto entity : scene->entities)
    {
        if (!entity->active) continue;

        if (entity->meshRenderer != nullptr)
        {
            auto mesh = entity->meshRenderer->mesh;

            if (mesh != nullptr)
            {
                for (int i = 0; i < mesh->submeshes.size(); ++i)
                {
                    Instance instance;
                    instance.transform = entity->transform;
                    instance.submesh = mesh->submeshes[i];
                    instance.material = nullptr;
                    if (i < entity->meshRenderer->materials.size()) {
                        instance.material = entity->meshRenderer->materials[i];
                    }
                    if (instance.material == nullptr) {
                        instance.material = resourceManager->materialWhite;
                    }
                    if (instance.material->shaderType == MaterialShaderType::Surface)
                    {
                        instanceArray.push_back(instance);
                    }
                }
            }
        }

        if (entity->lightSource != nullptr)
        {
            Instance instance;
            instance.transform = entity->transform;
            instance.transform->scale = QVector3D(0.1f, 0.1f, 0.1f);
            instance.submesh = resourceManager->sphere->submeshes[0];
            instance.material = resourceManager->materialLight;
            instanceArray.push_back(instance);
        }
    }

    // Sort geometry by material, then by submesh
    std::sort(instanceArray.begin(), instanceArray.end(), [](const Instance &d1, const Instance &d2) -> bool {
        return d1.material < d2.material || (d1.material == d2.material && d1.submesh < d2.submesh);
    });

    // Delete previous VAOs
    for (auto &instanceGroup : instanceGroupArray)
    {
        gl->glDeleteVertexArrays(1, &instanceGroup.vao);
        instanceGroup.vao = 0;
    }
    instanceGroupArray.clear();

    InstanceGroup instanceGroup;

    unsigned int offset = 0;

    // Populate instanceGroup with groups of instances
    for (auto &instance : instanceArray)
    {
        if (instance.submesh != instanceGroup.submesh ||instance.material != instanceGroup.material)
        {
            instanceGroup.submesh = instance.submesh;
            instanceGroup.material = instance.material;
            instanceGroup.modelViewMatrix.clear();
            instanceGroup.normalMatrix.clear();
            instanceGroup.count = 0;
            instanceGroup.offset = offset;
            instanceGroupArray.push_back(instanceGroup);
        }

        InstanceGroup &currentGroup = instanceGroupArray.back();
        currentGroup.modelViewMatrix.push_back(instance.transform->matrix());
        currentGroup.normalMatrix.push_back(instance.transform->matrix().normalMatrix());
        currentGroup.count++;

        offset += sizeof(QMatrix4x4) + sizeof(QMatrix3x3);
    }

    // Reserve vram for the instanced attributes
    unsigned int size = offset;
    if (size > instancingVBOSize || size < instancingVBOSize / 2)
    {
        instancingVBOSize = size;

        if (instancingVBO != 0)
        {
            gl->glDeleteBuffers(1, &instancingVBO);
            instancingVBO = 0;
        }

        if (instancingVBOSize > 0)
        {
            gl->glGenBuffers(1, &instancingVBO);
            gl->glBindBuffer(GL_ARRAY_BUFFER, instancingVBO);
            gl->glBufferData(GL_ARRAY_BUFFER, instancingVBOSize, nullptr, GL_STREAM_DRAW);
        }
    }

    if (instancingVBO == 0) return;

    // Upload instanced attributes
    gl->glBindBuffer(GL_ARRAY_BUFFER, instancingVBO);
    for (auto &instanceGroup : instanceGroupArray)
    {
        gl->glBufferSubData(GL_ARRAY_BUFFER, instanceGroup.offset, instanceGroup.count * sizeof(QMatrix4x4), &instanceGroup.modelViewMatrix[0]);
        gl->glBufferSubData(GL_ARRAY_BUFFER, instanceGroup.offset + instanceGroup.count * sizeof(QMatrix4x4), instanceGroup.count * sizeof(QMatrix3x3), &instanceGroup.normalMatrix[0]);
    }

    // Configure VAOs
    for (auto &instanceGroup : instanceGroupArray)
    {
        // VAO: Vertex format description and state of VBOs
        gl->glGenVertexArrays(1, &instanceGroup.vao);
        gl->glBindVertexArray(instanceGroup.vao);

        // Mesh attributes
        instanceGroup.submesh->enableAttributes();

        // Instanced attributes
        gl->glBindBuffer(GL_ARRAY_BUFFER, instancingVBO);
        // - modelview matrix
        gl->glEnableVertexAttribArray(5);
        gl->glEnableVertexAttribArray(6);
        gl->glEnableVertexAttribArray(7);
        gl->glEnableVertexAttribArray(8);
        gl->glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, sizeof(QMatrix4x4), (int*)(instanceGroup.offset + 0));
        gl->glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(QMatrix4x4), (int*)(instanceGroup.offset + 4*sizeof(float)));
        gl->glVertexAttribPointer(7, 4, GL_FLOAT, GL_FALSE, sizeof(QMatrix4x4), (int*)(instanceGroup.offset + 8*sizeof(float)));
        gl->glVertexAttribPointer(8, 4, GL_FLOAT, GL_FALSE, sizeof(QMatrix4x4), (int*)(instanceGroup.offset + 12*sizeof(float)));
        gl->glVertexAttribDivisor(5, 1);
        gl->glVertexAttribDivisor(6, 1);
        gl->glVertexAttribDivisor(7, 1);
        gl->glVertexAttribDivisor(8, 1);
        // - normal matrix
        gl->glEnableVertexAttribArray(9);
        gl->glEnableVertexAttribArray(10);
        gl->glEnableVertexAttribArray(11);
        unsigned int offset = instanceGroup.offset + instanceGroup.count * sizeof(QMatrix4x4);
        gl->glVertexAttribPointer(9,  3, GL_FLOAT, GL_FALSE, sizeof(QMatrix3x3), (void*)(offset + 0));
        gl->glVertexAttribPointer(10, 3, GL_FLOAT, GL_FALSE, sizeof(QMatrix3x3), (void*)(offset + 3*sizeof(float)));
        gl->glVertexAttribPointer(11, 3, GL_FLOAT, GL_FALSE, sizeof(QMatrix3x3), (void*)(offset + 6*sizeof(float)));
        gl->glVertexAttribDivisor(9, 1);
        gl->glVertexAttribDivisor(10, 1);
        gl->glVertexAttribDivisor(11, 1);

        // Release
        gl->glBindVertexArray(0);
        gl->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }

    gl->glBindBuffer(GL_ARRAY_BUFFER, 0);
}

QVector<QString> DeferredRenderer::getTextures() const
{
    return textures;
}

void DeferredRenderer::showTexture(QString textureName)
{
    m_shownTexture = textureName;
}

QString DeferredRenderer::shownTexture() const
{
    return m_shownTexture;
}

void DeferredRenderer::addTexture(QString textureName)
{
    if (textures.empty()) m_shownTexture = textureName;

    textures.push_back(textureName);
}

void DeferredRenderer::showLod(int lod)
{
    m_lod = lod;
}

int DeferredRenderer::shownLod() const
{
    return m_lod;
}

void DeferredRenderer::scheduleMousePicking(int mousex, int mousey)
{
    mousePickingEnabled = true;
    mousePickingMouseX = mousex;
    mousePickingMouseY = mousey;
}

bool DeferredRenderer::hasMousePickingResult(Entity **entity)
{
    if (mousePickingEntity != nullptr) *entity = mousePickingEntity;
    mousePickingEntity = nullptr;
    return (*entity != nullptr);
}

void DeferredRenderer::passMousePicking(Camera *camera)
{
    if (mousePickingEnabled == false) return;

    fboMousePicking->bind();

    GLenum drawBuffers[] = {GL_COLOR_ATTACHMENT0};
    gl->glDrawBuffers(1, drawBuffers);

    OpenGLState glState;
    glState.depthTest = true;
    glState.apply();

    gl->glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    gl->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    QOpenGLShaderProgram &program = mousePickingProgram->program;

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

                program.setUniformValue("worldMatrix", worldMatrix);
                program.setUniformValue("entityId", (float)meshRenderer->entity->id);

                for (auto submesh : mesh->submeshes)
                {
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
                worldMatrix.scale(0.1f, 0.1f, 0.1f);

                program.setUniformValue("worldMatrix", worldMatrix);
                program.setUniformValue("entityId", (float)lightSource->entity->id);

                resourceManager->sphere->submeshes[0]->draw();
            }
        }

        program.release();
    }

    float identifier = 0.0f;
    gl->glReadPixels(mousePickingMouseX,
                     viewportHeight - mousePickingMouseY - 1,
                     1, 1,
                     GL_RED, GL_FLOAT,
                     &identifier);

    //printf("id: %f\n", identifier);
    mousePickingEntity = scene->entityWithId(qRound(identifier));

    fboMousePicking->release();

    mousePickingEnabled = false;
}
