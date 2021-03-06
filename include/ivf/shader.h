#ifndef __ivf2_shader__
#define __ivf2_shader__

#include <ivf/glbase.h>

#include <string>

namespace ivf {
    
    class Shader : public GLBase {
    protected:
        std::string m_filename;
        std::string m_shaderCode;
        GLuint m_id;
    public:
        Shader(const std::string filename="");
        virtual ~Shader();

        static std::shared_ptr<Shader> create(const std::string filename = "");
        
        void setFilename(const std::string filename);
        const std::string filename();
                
        bool read();
        
        virtual bool compile();
        
        GLuint id();
    };

    typedef std::shared_ptr<Shader> ShaderPtr;

};

#endif
