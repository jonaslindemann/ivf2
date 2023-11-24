#pragma once

#include <ivf/glbase.h>

namespace ivf {
    
    class VertexArray : public GLBase {
    private:
        GLuint m_id;
    public:
        VertexArray();
        virtual ~VertexArray();

        static std::shared_ptr<VertexArray> create();
        
        void bind();
        void unbind();
    };

    typedef std::shared_ptr<VertexArray> VertexArrayPtr;

};