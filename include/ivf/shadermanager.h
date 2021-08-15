#ifndef __ivf2_shader_manager__
#define __ivf2_shader_manager__

#include <ivf/vertexshader.h>
#include <ivf/fragmentshader.h>
#include <ivf/program.h>
#include <ivf/transformmanager.h>

#include <string>
#include <vector>
#include <memory>
#include <map>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace ivf {
    
    class ShaderManager {
    private:
        ShaderManager();
        static ShaderManager* m_instance;

        bool m_linkErrors;
        bool m_vertexCompileErrors;
        bool m_fragCompileErrors;

        std::map<std::string, std::shared_ptr<Program>> m_programs;
        std::shared_ptr<Program> m_currentProgram;
    
    public:
        static ShaderManager* instance()
        {
            if (!m_instance)
            {
                if (!m_instance)
                    m_instance = new ShaderManager();
            }
            return m_instance;
        }

        static ShaderManager* create()
        {
            if (!m_instance)
            {
                if (!m_instance)
                    m_instance = new ShaderManager();
            }
            return m_instance;
        }

        static void drop()
        {
            delete m_instance;
            m_instance = 0;
        }
    
        std::shared_ptr<Program> loadProgramFromFiles(const std::string vertexShader, const std::string fragmentShader, const std::string name);
        std::shared_ptr<Program> currentProgram();
    
        void apply();

        bool compileLinkErrors();
    
    };

    typedef ShaderManager* ShaderManagerPtr;
    
}

#endif
