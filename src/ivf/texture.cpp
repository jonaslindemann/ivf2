#include <ivf/texture.h>

#include <ivf/shadermanager.h>

#include <ivf/utils.h>

#include <iostream>

using namespace ivf;
using namespace std;

Texture::Texture()
	:m_id(0),
	 m_wrapT(GL_REPEAT),
	 m_wrapS(GL_REPEAT),
	 m_minFilter(GL_LINEAR_MIPMAP_LINEAR),
	 m_magFilter(GL_LINEAR),
	 m_intFormat(GL_RGB),
	 m_level(0),
	 m_format(GL_RGBA),
	 m_type(GL_UNSIGNED_BYTE)
{
	glGenTextures(1, &m_id);
}

Texture::~Texture()
{
	glDeleteTextures(1, &m_id);
}

std::shared_ptr<Texture> Texture::create()
{
	return std::make_shared<Texture>();
}

void Texture::bind()
{
	GL_ERR_BEGIN;
	glBindTexture(GL_TEXTURE_2D, m_id);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, m_wrapS);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, m_wrapT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, m_minFilter);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, m_magFilter);
	GL_ERR_END("Texture::bind()");
}

bool Texture::load(const std::string filename)
{ 
	int width, height, nrChannels;
	unsigned char* data = stbi_load(filename.c_str(), &width, &height, &nrChannels, 0);

	cout << "Image channels: " << nrChannels << endl;

	if (data)
	{
		this->bind();
		GL_ERR_BEGIN;
		glTexImage2D(GL_TEXTURE_2D, m_level, m_intFormat, width, height, 0, m_format, m_type, data);
		glGenerateMipmap(GL_TEXTURE_2D);
		GL_ERR_END("Texture::load()");
		this->unbind();
	}
	else
	{
		stbi_image_free(data);
		std::cout << "Failed to load texture" << std::endl;
		return false;
	}
	stbi_image_free(data);

	return true;
}

void Texture::unbind()
{
	GL_ERR( glBindTexture(GL_TEXTURE_2D, 0) );
}

void ivf::Texture::setIntFormat(GLint format)
{
	m_intFormat = format;
}

void ivf::Texture::setFormat(GLenum format)
{
	m_format = format;
}

void ivf::Texture::setType(GLenum type)
{
	m_type = type;
}

void ivf::Texture::setLevel(GLint level)
{
	m_level = level;
}

GLuint ivf::Texture::id()
{
	return m_id;
}
