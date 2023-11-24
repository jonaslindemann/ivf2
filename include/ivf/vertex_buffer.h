#pragma once

#include <ivf/glbase.h>
#include <ivf/field.h>

namespace ivf {
    
    class VertexBuffer : public GLBase {
    private:
        GLuint m_id;
    public:
        VertexBuffer();
        virtual ~VertexBuffer();

        static std::shared_ptr<VertexBuffer> create();
        
        void bind();
        void unbind();
        
        void setArray(Field* field);
    };

    typedef std::shared_ptr<VertexBuffer> VertexBufferPtr;

};
