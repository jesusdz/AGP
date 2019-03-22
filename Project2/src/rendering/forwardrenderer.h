#ifndef FORWARDRENDERER_H
#define FORWARDRENDERER_H

#include "renderer.h"

class ShaderProgram;

class ForwardRenderer : public Renderer
{
public:
    ForwardRenderer();

    void initialize() override;
    void resize(int width, int height) override;
    void render(Camera *camera) override;

private:

    void passMeshes(Camera *camera);
    void passTerrains(Camera *camera);
    void passGrid(Camera *camera);

    ShaderProgram *gridProgram;
};

#endif // FORWARDRENDERER_H
