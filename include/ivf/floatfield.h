#ifndef __ivf2_floatfield__
#define __ivf2_floatfield__

#include <ivf/field.h>

#include <glad/glad.h>


namespace ivf {
    
    class FloatField : public Field {
    private:
        GLfloat* m_data;
    public:
        FloatField(GLuint rows, GLuint cols);
        virtual ~FloatField();

        static std::shared_ptr<FloatField> create(GLuint rows, GLuint cols);
        
        GLfloat at(GLuint rows, GLuint cols);
        GLfloat at(GLuint pos);
        
        void set(GLuint rows, GLuint cols, GLfloat value);
        GLfloat get(GLuint rows, GLuint cols);
        
        virtual size_t memSize();
        
        virtual void zero();
        
        virtual void* data();
		virtual GLenum dataType();
    };

    typedef std::shared_ptr<FloatField> FloatFieldPtr;
    
};

#endif
