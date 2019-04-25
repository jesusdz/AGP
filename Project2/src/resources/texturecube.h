#ifndef TEXTURECUBE_H
#define TEXTURECUBE_H

#include "resource.h"
#include "rendering/gl.h"


class TextureCube : public Resource
{
public:

    static const char *TypeName;

    TextureCube();
    ~TextureCube() override;

    const char *typeName() const override { return TypeName; }

    TextureCube * asTextureCube() override { return this; }

    void update() override;
    void destroy() override;

    void bind(unsigned int textureUnit);

    void read(const QJsonObject &json) override { }
    void write(QJsonObject &json) override { }

    GLuint textureId() const { return id; }

    int resolution = 512;
    bool generateMipMap = false;

private:

    GLuint id = 0;
};

#endif // TEXTURECUBE_H
