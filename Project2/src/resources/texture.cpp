#include "texture.h"
#include <QJsonObject>
#include <QVector2D>


const char *Texture::TypeName = "Texture";


Texture::Texture() :
    tex(QOpenGLTexture::Target2D),
    wrapMode(QOpenGLTexture::WrapMode::Repeat)
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
    tex.setMinificationFilter(QOpenGLTexture::LinearMipMapLinear);
    tex.setMagnificationFilter(QOpenGLTexture::Linear);
    tex.setWrapMode(wrapMode);
    if (!image.isNull())
    {
        const bool opengl = true;
        if (opengl)
        {
            QImage flipped = image.mirrored();
            tex.setData(flipped);
        }
        else
        {
            tex.setData(image);
        }
    }
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

    filePath = QString::fromLatin1(filename);
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

QVector2D Texture::size() const
{
    return QVector2D(image.width(), image.height());
}

void Texture::read(const QJsonObject &json)
{
    filePath = absolutePathInProject(json["filePath"].toString());
    if (!filePath.isEmpty())
    {
        loadTexture(filePath.toLatin1());
    }
}

void Texture::write(QJsonObject &json)
{
    json["filePath"] = relativePathInProject(filePath);
}
