#ifndef TEXTURE_H
#define TEXTURE_H

#include "resource.h"
#include <QOpenGLTexture>
#include <QImage>


class Texture : public Resource
{
public:

    static const char *TypeName;

    Texture();
    ~Texture() override;

    const char *typeName() const override { return TypeName; }

    Texture * asTexture() override { return this; }

    void update() override;
    void destroy() override;

    void bind(unsigned int textureUnit);

    void loadTexture(const char *filename);
    void setImage(const QImage &img);
    void setWrapMode(QOpenGLTexture::WrapMode wrap);
    int width() const;
    int height() const;

    void read(const QJsonObject &json) override;
    void write(QJsonObject &json) override;

private:

    QOpenGLTexture tex;
    QImage image;
    QOpenGLTexture::WrapMode wrapMode;

    QString filePath;
};

#endif // TEXTURE_H
