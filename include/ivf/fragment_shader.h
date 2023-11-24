#pragma once 

#include <ivf/shader.h>

#include <string>

namespace ivf {
    
    class FragmentShader : public Shader {
    private:
    public:
        FragmentShader(const std::string filename="");
        virtual ~FragmentShader();

        static std::shared_ptr<FragmentShader> create(const std::string filename = "");
        
        virtual bool compile();
    };

    typedef std::shared_ptr<FragmentShader> FragmentShaderPtr;

};