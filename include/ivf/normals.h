#ifndef __arc2_normals__
#define __arc2_normals__

#include <ivf/floatfield.h>

#include <glm/glm.hpp>

namespace ivf {
    
    class Normals : public FloatField {
    public:
        Normals(GLuint nNormals);

        static std::shared_ptr<Normals> create(GLuint nNormals);
        
        void setNormal(GLuint idx, GLfloat nx, GLfloat ny, GLfloat nz);
        void getNormal(GLuint idx, GLfloat& nx, GLfloat& ny, GLfloat& nz);

        glm::vec3 normal(GLuint idx);
        void setNormal(GLuint idx, glm::vec3 normal);
    };

    typedef std::shared_ptr<Normals> NormalsPtr;
    
};


#endif