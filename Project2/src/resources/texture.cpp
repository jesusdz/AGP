#include "texture.h"


Texture::Texture() :
    tex(QOpenGLTexture::Target2D),
    wrapMode(QOpenGLTexture::WrapMode::ClampToEdge)
{
    image = QImage(1, 1, QImage::Format::Format_RGB888);
    image .setPixelColor(0, 0, QColor::fromRgb(255, 0, 255));
    needsUpdate = true;
}

Texture::~Texture()
{

}

void Texture::update()
{
    if (tex.isCreated()) {
        tex.destroy();
    }

    // Decide the pixel format
    //QOpenGLTexture::PixelFormat pixelFormat;
    // TODO: pixelFormat based on what the user wants
    //tex.allocateStorage(pixelFormat, QOpenGLTexture::PixelType::UInt8);

    // Create and upload the texture
    tex.create();
    tex.setMinMagFilters(QOpenGLTexture::Filter::Linear, QOpenGLTexture::Filter::Linear);
    tex.setWrapMode(wrapMode);
    if (!image.isNull())
    {
        tex.setData(image);
    }

    needsUpdate = false;
}

void Texture::destroy()
{
    tex.destroy();
}

void Texture::bind(unsigned int textureUnit)
{
    tex.bind(textureUnit);
}

void Texture::loadTexture(const char *filename)
{
    image = QImage(QString::fromLatin1(filename));

    if (image.isNull())
    {
        qDebug("Could not open image %s in Texture::loadTexture()", filename);
        return;
    }

    needsUpdate = true;
}

void Texture::setImage(const QImage &img)
{
    image = img;
    needsUpdate = true;
}

void Texture::setWrapMode(QOpenGLTexture::WrapMode wrap)
{
    wrapMode = wrap;
}

int Texture::width() const
{
    return image.width();
}

int Texture::height() const
{
    return image.height();
}


void Texture::read(const QJsonObject &json)
{
    // TODO
}

void Texture::write(QJsonObject &json)
{
    // TODO
}
