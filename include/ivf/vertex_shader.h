#pragma once

#include <ivf/shader.h>

#include <string>

namespace ivf {
    
    class VertexShader : public Shader {
    private:
        
    public:
        VertexShader(const std::string filename="");
        virtual ~VertexShader();

        static std::shared_ptr<VertexShader> create(const std::string filename = "");
        
        virtual bool compile();
    };

    typedef std::shared_ptr<VertexShader> VertexShaderPtr;

};
