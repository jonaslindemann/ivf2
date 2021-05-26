#ifndef __ivf2_intfield__
#define __ivf2_intfield__

#include <ivf/field.h>

#include <glad/glad.h>

namespace ivf {
    
	class IntField : public Field {
    private:
        GLuint* m_data;
    public:
		IntField(GLuint rows, GLuint cols);
		virtual ~IntField();

        static std::shared_ptr<IntField> create(GLuint rows, GLuint cols);
        
		GLuint at(GLuint row, GLuint col);
		GLuint at(GLuint pos);
        
        void set(GLuint row, GLuint col, GLuint value);
        
        virtual size_t memSize();
        
        virtual void zero();

		virtual GLenum dataType();
        
        virtual void* data();
    };
    typedef std::shared_ptr<IntField> IntFieldPtr;
    
};

#endif
