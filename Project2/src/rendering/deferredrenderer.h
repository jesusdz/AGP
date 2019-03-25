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
    void passBlit();

    ShaderProgram *materialProgram = nullptr;
    ShaderProgram *blitProgram = nullptr;

    // **** Render targets ****
    // rt0: Albedo (RGB), occlussion (A)
    // rt1: Specular (RGB), roughness (A)
    // rt2: World normal (RGB), unused (A)
    // rt3: Emission + lightmaps (RGB)
    // rt4: Depth + stencil
    GLuint rt0 = 0;
    GLuint rt1 = 0;
    GLuint rt2 = 0;
    GLuint rt3 = 0;
    GLuint rt4 = 0;
    FramebufferObject *fbo = nullptr;
};

#endif // DEFERREDRENDERER_H
