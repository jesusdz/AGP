#include "renderer.h"

QVector<QString> Renderer::getTextures() const
{
    QVector<QString> textureNames;
    for (auto &texture : textures)
    {
        textureNames.push_back(texture.first);
    }
    return textureNames;
}

void Renderer::showTexture(QString textureName)
{
    for (int i = 0; i < textures.size(); ++i)
    {
        if (textures[i].first == textureName)
        {
            m_shownTexture = i;
        }
    }
}

unsigned int Renderer::shownTexture() const
{
    return textures[m_shownTexture].second;
}

void Renderer::addTexture(QString textureName)
{
    textures.push_back({textureName, 0});
}

void Renderer::setTexture(QString textureName, unsigned int identifier)
{
    for (auto &texture : textures)
    {
        if (texture.first == textureName)
        {
            texture.second = identifier;
        }
    }
}
