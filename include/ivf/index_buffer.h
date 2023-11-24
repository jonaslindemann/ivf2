#pragma once

#include <ivf/glbase.h>
#include <ivf/field.h>

namespace ivf {
    
	class IndexBuffer : public GLBase {
    private:
        GLuint m_id;
    public:
		IndexBuffer();
		virtual ~IndexBuffer();

        static std::shared_ptr<IndexBuffer> create();
        
        void bind();
        void unbind();
        
        void setArray(Field* field);
    };

    typedef std::shared_ptr<IndexBuffer> IndexBufferPtr;
};
