#ifndef DEFERREDRENDERER_H
#define DEFERREDRENDERER_H

#include "renderer.h"
#include "gl.h"

class ShaderProgram;
class FramebufferObject;

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

    void passMeshes(Camera *camera);
    void passLights(Camera *camera);
    void passGrid(Camera *camera);
    void passBlit();

    float viewportWidth = 128.0;
    float viewportHeight = 128.0;

    ShaderProgram *materialProgram = nullptr;
    ShaderProgram *lightingProgram = nullptr;
    ShaderProgram *gridProgram = nullptr;
    ShaderProgram *blitProgram = nullptr;

    // **** Render targets ****
    // rt0: Albedo (RGB), occlussion (A)
    // rt1: Specular (RGB), roughness (A)
    // rt2: World normal (RGB), unused (A)
    // rt3: Light (Emission + lightmaps + lighting pass) (RGB), unused (A)
    // rt4: Depth + stencil
    GLuint rt0 = 0;
    GLuint rt1 = 0;
    GLuint rt2 = 0;
    GLuint rt3 = 0;
    GLuint rt4 = 0;
    FramebufferObject *fbo = nullptr;
};

#endif // DEFERREDRENDERER_H
