#ifndef FORWARDRENDERER_H
#define FORWARDRENDERER_H

#include "renderer.h"

class ForwardRenderer : public Renderer
{
public:
    ForwardRenderer();

    void resize(int width, int height) override;
    void render(Camera *camera) override;
};

#endif // FORWARDRENDERER_H
