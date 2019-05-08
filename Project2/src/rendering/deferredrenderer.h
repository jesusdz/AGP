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

    void generateEnvironments();
    void clearBuffers();
    void passMeshes(Camera *camera);
    void passSSAO(Camera *camera);
    void passSSAOBlur();
    void passLights(Camera *camera);
    void passBackground(Camera *camera, GLenum colorAttachment);
    enum class WaterScenePart { Reflection, Refraction };
    void passWaterScene(Camera *camera, GLenum colorAttachment, WaterScenePart part);
    void passWaterPlane(Camera *camera, GLenum colorAttachment);
    void passSelectionMask(Camera *camera, GLenum colorAttachment);
    void passSelectionOutline(Camera *camera, GLenum colorAttachment);
    void passGrid(Camera *camera, GLenum colorAttachment);
    void passMotionBlur(Camera *camera);
	void passBlur(Camera *Camera, FramebufferObject *fbo, GLenum colorAttachment, GLuint inputTexture, GLint inputLod);
    void passBlit();

    float viewportWidth = 128.0;
    float viewportHeight = 128.0;

    ShaderProgram *forwardWithClippingProgram = nullptr;
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
    ShaderProgram *motionBlurProgram = nullptr;
    ShaderProgram *blitProgram = nullptr;
    ShaderProgram *blitCubeProgram = nullptr;
    ShaderProgram *waterProgram = nullptr;
	ShaderProgram *blur = nullptr;

    // **** Render targets ****
    GLuint rt0 = 0; // Albedo (RGB), occlussion (A)
    GLuint rt1 = 0; // Specular (RGB), roughness (A)
    GLuint rt2 = 0; // World normal (RGB), unused (A)
    GLuint rt3 = 0; // Light (Emission + lightmaps + lighting pass) (RGB), unused (A)
    GLuint rt5 = 0; // Tmp RGBA
    GLuint rtD = 0; // Depth + stencil

    FramebufferObject *fbo = nullptr;
    FramebufferObject *fbo1 = nullptr;
    FramebufferObject *fbo2 = nullptr;
    FramebufferObject *fbo3 = nullptr;
    FramebufferObject *fbo4 = nullptr;

    // **** Water render targets ****
    GLuint rtReflection = 0;
    GLuint rtRefraction = 0;
    GLuint rtReflectionDepth = 0;
    GLuint rtRefractionDepth = 0;

    FramebufferObject *fboReflection = nullptr;
    FramebufferObject *fboRefraction = nullptr;

    // SSAO stuff //////////////////////////////////////////////////////

    QVector<QVector3D> ssaoKernel; // Samples for the SSAO technique
    GLuint ssaoNoiseTex = 0;       // Noise texture for SSAO

    // Instancing stuff ////////////////////////////////////////////////

    struct Instance
    {
        Transform *transform = nullptr;
        SubMesh *submesh = nullptr;
        Material *material = nullptr;
    };

    QVector<Instance> instanceArray;
    bool mustUpdateInstances = false;

    void updateRenderList() override;

    struct InstanceGroup
    {
        SubMesh *submesh = nullptr;
        Material *material = nullptr;
        QVector<QMatrix4x4> modelViewMatrix;
        QVector<QMatrix3x3> normalMatrix;
        unsigned int count = 0;
        unsigned int offset = 0;
        GLuint vao = 0;
    };

    QVector<InstanceGroup> instanceGroupArray;

    GLenum instancingVBO = 0;
    unsigned int instancingVBOSize = 0;

    void updateRenderListIntoGPU();
};

#endif // DEFERREDRENDERER_H
