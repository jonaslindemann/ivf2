#pragma once

#include <ivf/float_field.h>

#include <glm/glm.hpp>

namespace ivf {
    
    class Vertices : public FloatField {
    public:
        Vertices(GLuint nVertices);

        static std::shared_ptr<Vertices> create(GLuint nVertices);
        
        void setVertex(GLuint idx, GLfloat x, GLfloat y, GLfloat z);
		void getVertex(GLuint idx, GLfloat& x, GLfloat& y, GLfloat& z);

        glm::vec3 vertex(GLuint idx);
    };

    typedef std::shared_ptr<Vertices> VerticesPtr;
    
};
