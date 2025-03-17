#include "Texture2D.h"
#include "stb_image.h"


Texture2D::Texture2D(const std::string& path)
{
    GLCALL(glGenTextures(1, &m_id));
    GLCALL(glBindTexture(GL_TEXTURE_2D, m_id));

    GLCALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER));	
    GLCALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER));

    GLCALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
    GLCALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
    //GLCALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR));
    //GLCALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));

    unsigned char *data = stbi_load(path.c_str(), &m_width, &m_height, &m_bpp, 0);

    GLCALL(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_width, m_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data));
    GLCALL(glGenerateMipmap(GL_TEXTURE_2D));

    if(data) stbi_image_free(data);
}

Texture2D::~Texture2D()
{
    GLCALL(glDeleteTextures(1, &m_id));
}

void Texture2D::bind(u32 slot) const
{
    GLCALL(glActiveTexture(GL_TEXTURE0 + slot));
    GLCALL(glBindTexture(GL_TEXTURE_2D, m_id));
}

void Texture2D::unbind() const
{
    GLCALL(glBindTexture(GL_TEXTURE_2D, 0));
}

