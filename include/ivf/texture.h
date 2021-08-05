#pragma once

#include <string>

#include <ivf/glbase.h>
#include <ivf/field.h>
#include <stb/stb_image.h>

namespace ivf {

    class Texture : public GLBase {
    private:
        GLuint m_id;
        GLint m_wrapT;
        GLint m_wrapS;
        GLint m_minFilter;
        GLint m_magFilter;
        GLint m_intFormat;
        GLint m_level;
        GLint m_texUnit;
        GLenum m_format;
        GLenum m_type;
    public:
        Texture();
        virtual ~Texture();

        static std::shared_ptr<Texture> create();

        void bind();
        bool load(const std::string filename);
        void unbind();

        void setIntFormat(GLint format);
        void setFormat(GLenum format);
        void setType(GLenum type);
        void setLevel(GLint level);
        void setTexUnit(GLint unit);

        GLint texUnit();

        GLuint id();
    };

    typedef std::shared_ptr<Texture> TexturePtr;

};
