#ifndef RENDERER_H
#define RENDERER_H

class Camera;

class Renderer
{
public:
    Renderer() { }
    virtual ~Renderer() { }

    virtual void resize(int width, int height) = 0;
    virtual void render(Camera *camera) = 0;
};

#endif // RENDERER_H
