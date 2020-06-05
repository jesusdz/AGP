#ifndef DEFERREDRENDERER_H
#define DEFERREDRENDERER_H

#include "gl.h"
#include "gldebug.h"
#include <QMatrix3x3>

class ShaderProgram;
class FramebufferObject;
class Transform;
class SubMesh;
class Material;
class Camera;
class Entity;

class DeferredRenderer
{
public:
    DeferredRenderer();
    ~DeferredRenderer();

    void initialize();
    void finalize();

    void resize(int width, int height);
    void render(Camera *camera);

    void scheduleMousePicking(int mousex, int mousey);
    bool hasMousePickingResult(Entity **entity);

    QVector<QString> getTextures() const;
    void showTexture(QString textureName);
    QString shownTexture() const;

    void showLod(int lod);
    int shownLod() const;

private:

    bool mousePickingEnabled = false;
    Entity *mousePickingEntity = nullptr;
    int mousePickingMouseX = -1;
    int mousePickingMouseY = -1;

    void addTexture(QString textureName);
    QVector<QString> textures;
    QString m_shownTexture;
    int m_lod = 0;

    void generateEnvironments();
    void clearBuffers();
    void passMeshes(Camera *camera);
    void passSSAO(Camera *camera);
    void passSSAOBlur();
    void passShadowmaps(Camera *camera);
    void passLights(Camera *camera);
    void passBackground(Camera *camera, GLenum colorAttachment);
    enum class WaterScenePart { Reflection, Refraction };
    void passWaterScene(Camera *camera, GLenum colorAttachment, WaterScenePart part);
    void passWaterPlane(Camera *camera, GLenum colorAttachment);
    void passSelectionMask(Camera *camera, GLenum colorAttachment);
    void passSelectionOutline(Camera *camera, GLenum colorAttachment);
    void passGrid(Camera *camera, GLenum colorAttachment);
    void passMotionBlur(Camera *camera);
    void passBlitBrightPixels(FramebufferObject *fbo, const QVector2D &viewportSize, GLenum colorAttachment, GLuint inputTexture, GLint inputLod, float threshold);
    void passBlur(FramebufferObject *fbo, const QVector2D &viewportSize, GLenum colorAttachment, GLuint inputTexture, GLint inputLod, const QVector2D &direction, float blurRadies);
    void passBloom(FramebufferObject *fbo, GLenum colorAttachment, GLuint inputTexture, int maxLod);
    void passBlit();
    void passDebug();
    void passMousePicking(Camera *camera);

    float viewportWidth = 128.0;
    float viewportHeight = 128.0;

    ShaderProgram *forwardWithClippingProgram = nullptr;
    ShaderProgram *environmentToCubemapProgram = nullptr;
    ShaderProgram *irradianceProgram = nullptr;
    ShaderProgram *materialProgram = nullptr;
    ShaderProgram *ssaoProgram = nullptr;
    ShaderProgram *ssaoBlurProgram = nullptr;
    ShaderProgram *shadowmapProgram = nullptr;
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
    ShaderProgram *blitBrightestPixelsProgram = nullptr;
    ShaderProgram *blur = nullptr;
    ShaderProgram *bloomProgram = nullptr;
    ShaderProgram *mousePickingProgram = nullptr;

    // **** Render targets ****
    GLuint rt0 = 0; // Albedo (RGB), occlussion (A)
    GLuint rt1 = 0; // Specular (RGB), roughness (A)
    GLuint rt2 = 0; // World normal (RGB), unused (A)
    GLuint rt3 = 0; // Light (Emission + lightmaps + lighting pass) (RGB), unused (A)
    GLuint rt5 = 0; // Tmp RGBA
    GLuint rtD = 0; // Depth + stencil

    FramebufferObject *fbo = nullptr;

    // **** Bloom mipmap ****
    GLuint rtBright; // For blitting brightest pixels and vertical blur
    GLuint rtBloomH; // For first pass horizontal blur
    FramebufferObject *fboBloom1 = nullptr;
    FramebufferObject *fboBloom2 = nullptr;
    FramebufferObject *fboBloom3 = nullptr;
    FramebufferObject *fboBloom4 = nullptr;
    FramebufferObject *fboBloom5 = nullptr;

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

    // Environment stuff ///////////////////////////////////////////////

    GLuint environmentMap = 0;
    GLuint irradianceMap = 0;
    GLuint environmentMapResolution = 512;
    GLuint irradianceMapResolution = 32;

    // MousePicking stuff //////////////////////////////////////////////

    GLuint rtMousePicking = 0;
    FramebufferObject *fboMousePicking = nullptr;


    // Instancing stuff ////////////////////////////////////////////////

    struct Instance
    {
        Transform *transform = nullptr;
        SubMesh *submesh = nullptr;
        Material *material = nullptr;
    };

    QVector<Instance> instanceArray;

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

    // SHADOWS /////////////////////////////////////////////////////////

    GLuint shadowMap;
    FramebufferObject *fboShadowmap = nullptr;
    unsigned int shadowMapWidth = 512;
    unsigned int shadowMapHeight = 512;


    // TODO(jesus): Declare texture mipmap for directional light shadows


    // DEBUG ///////////////////////////////////////////////////////////

    GLDebugContext glDebug;
    ShaderProgram *debugProgram = nullptr;
    ShaderProgram *debugTextProgram = nullptr;
    GLuint fontTexture;


    // QUERIES /////////////////////////////////////////////////////////

    enum {
        QUERY_RENDER,
        QUERY_COUNT
    };
    GLuint queries[QUERY_COUNT * 2];
    GLuint *queriesBack = nullptr;
    GLuint *queriesFront = nullptr;
};

#endif // DEFERREDRENDERER_H
