#ifndef __ivf2_program__
#define __ivf2_program__

#include <ivf/glbase.h>
#include <ivf/shader.h>

#include <string>
#include <vector>
#include <memory>

#include <glad/glad.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace ivf {
    
    class Program : public GLBase {
    protected:
        std::vector<std::shared_ptr<Shader>> m_shaders;
        GLuint m_id;
    public:
        Program();
        virtual ~Program();

        static std::shared_ptr<Program> create();
        
        void addShader(std::shared_ptr<Shader> shader);
        void clear();
        size_t shaderCount();
        
        bool link();
        void use();

        void bindAttribLoc(GLuint idx, const std::string name);
        
        GLint attribId(const std::string name);
        GLint uniformLoc(const std::string name);
        
        void uniformMatrix4(const std::string name, glm::mat4 matrix);
		void uniformBool(const std::string name, bool flag);
        
        void uniformVec4f(const std::string name, float v0, float v1, float v2, float v3);
        void uniformVec3f(const std::string name, float v0, float v1, float v2);
        void uniformVec3(const std::string name, const glm::vec3 v);
        void uniformVec4(const std::string name, const glm::vec4 v);

        void printAttribs();
        
        GLuint id();
    };

    typedef std::shared_ptr<Program> ProgramPtr;

};

#endif
