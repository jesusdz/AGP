#ifndef DEFERREDRENDERER_H
#define DEFERREDRENDERER_H

#include "renderer.h"
#include "gl.h"
#include <QMatrix3x3>

class ShaderProgram;
class FramebufferObject;
class Transform;
class SubMesh;
class Material;

class DeferredRenderer : public Renderer
{
public:
    DeferredRenderer();
    ~DeferredRenderer() override;

    void initialize() override;
    void finalize() override;

    void resize(int width, int height) override;
    void render(Camera *camera) override;

private:

    void passEnvironments();
    void passMeshes(Camera *camera);
    void passSSAO(Camera *camera);
    void passSSAOBlur();
    void passLights(Camera *camera);
    void passBackground(Camera *camera);
    void passSelectionOutline(Camera *camera);
    void passGrid(Camera *camera);
    void passMotionBlur(Camera *camera);
    void passBlit();

    float viewportWidth = 128.0;
    float viewportHeight = 128.0;

    ShaderProgram *equirectangularToCubemapProgram = nullptr;
    ShaderProgram *irradianceProgram = nullptr;
    ShaderProgram *materialProgram = nullptr;
    ShaderProgram *ssaoProgram = nullptr;
    ShaderProgram *ssaoBlurProgram = nullptr;
    ShaderProgram *lightingProgram = nullptr;
    ShaderProgram *backgroundProgram = nullptr;
    ShaderProgram *boxProgram = nullptr;
    ShaderProgram *selectionMaskProgram = nullptr;
    ShaderProgram *selectionOutlineProgram = nullptr;
    ShaderProgram *gridProgram = nullptr;
    ShaderProgram *blurProgram = nullptr;
    ShaderProgram *blitProgram = nullptr;
    ShaderProgram *blitCubeProgram = nullptr;

    // **** Render targets ****
    GLuint rt0 = 0; // Albedo (RGB), occlussion (A)
    GLuint rt1 = 0; // Specular (RGB), roughness (A)
    GLuint rt2 = 0; // World normal (RGB), unused (A)
    GLuint rt3 = 0; // Light (Emission + lightmaps + lighting pass) (RGB), unused (A)
    GLuint rt4 = 0; // Depth + stencil
    GLuint rt5 = 0; // Tmp RGBA

    FramebufferObject *fbo = nullptr;

    QVector<QVector3D> ssaoKernel; // Samples for the SSAO technique
    GLuint ssaoNoiseTex = 0;       // Noise texture for SSAO

    // Instancing stuff ////////////////////////////////////////////////

    struct RenderData
    {
        Transform *transform = nullptr;
        SubMesh *submesh = nullptr;
        Material *material = nullptr;
    };

    QVector<RenderData> renderDataArray;
    bool renderDataArrayUpdated = false;

    void updateRenderList() override;

    struct InstanceData
    {
        SubMesh *submesh = nullptr;
        Material *material = nullptr;
        QVector<QMatrix4x4> modelViewMatrix;
        QVector<QMatrix3x3> normalMatrix;
        unsigned int instance_count = 0;
        GLuint vao = 0;
        GLuint vbo = 0;
    };

    QVector<InstanceData> instanceDataArray;

    void updateRenderListIntoGPU();
};

#endif // DEFERREDRENDERER_H
