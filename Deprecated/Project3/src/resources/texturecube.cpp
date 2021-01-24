#include "texturecube.h"


const char *TextureCube::TypeName = "TextureCube";


TextureCube::TextureCube()
{
    needsUpdate = true;
}

TextureCube::~TextureCube()
{
}

void TextureCube::update()
{
    if (id != 0) {
        gl->glDeleteTextures(1, &id);
    }
    gl->glGenTextures(1, &id);
    gl->glBindTexture(GL_TEXTURE_CUBE_MAP, id);


    for (unsigned int i = 0; i < 6; ++i)
    {
        // note that we store each face with 16 bit floating point values
        gl->glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                     0, GL_RGB16F,
                     resolution, resolution, 0,
                     GL_RGB, GL_FLOAT, nullptr);
    }

    gl->glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    gl->glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    gl->glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    GLenum filter = generateMipMap ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR;
    gl->glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, filter);
    gl->glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    if (generateMipMap)
    {
        gl->glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
    }
}

void TextureCube::destroy()
{
    gl->glDeleteTextures(1, &id);
}

void TextureCube::bind(unsigned int textureUnit)
{
    gl->glActiveTexture(GL_TEXTURE0 + textureUnit);
    gl->glBindTexture(GL_TEXTURE_CUBE_MAP, id);
}
