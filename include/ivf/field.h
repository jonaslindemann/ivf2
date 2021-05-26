#ifndef __ivf2_field__
#define __ivf2_field__

#include <ivf/base.h>

#include <cstdlib>

#include <glad/glad.h>

namespace ivf {
      
    class Field : public Base {
    protected:
        GLuint m_size[2];
    public:
        Field(GLuint rows, GLuint cols);
        virtual ~Field();

        static std::shared_ptr<Field> create(GLuint rows, GLuint cols);
        
        GLuint rows();
        GLuint cols();
        GLuint size();
        
        virtual void zero();
        virtual size_t memSize();
        
        virtual void* data();
		virtual GLenum dataType();
    };

    typedef std::shared_ptr<Field> FieldPtr;
    
};

#endif
