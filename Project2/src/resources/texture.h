#ifndef TEXTURE_H
#define TEXTURE_H

#include "resource.h"
#include <QOpenGLTexture>
#include <QImage>


class Texture : public Resource
{
public:
    Texture();
    ~Texture() override;

    Texture * asTexture() override { return this; }

    void update() override;
    void destroy() override;

    void bind(unsigned int textureUnit);

    void loadTexture(const char *filename);
    void setImage(const QImage &img);
    void setWrapMode(QOpenGLTexture::WrapMode wrap);
    int width() const;
    int height() const;

private:

    QOpenGLTexture tex;
    QImage image;
    QOpenGLTexture::WrapMode wrapMode;
};

#endif // TEXTURE_H
