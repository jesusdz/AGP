#ifndef TEXTURE_H
#define TEXTURE_H

#include "resource.h"

class Texture : public Resource
{
public:
    Texture();
    ~Texture() override;

    Texture * asTexture() override { return this; }

    void update() override;
    void destroy() override;

    void loadTexture(const char *filename);
};

#endif // TEXTURE_H
