#pragma once
#include <ivf/floatfield.h>

#include <glm/glm.hpp>

namespace ivf {
    
    class TexCoords : public FloatField {
    public:
        TexCoords(GLuint nVertices);

        static std::shared_ptr<TexCoords> create(GLuint nVertices);
        
        void setTexCoord(GLuint idx, GLfloat s, GLfloat t);
		void getTexCoord(GLuint idx, GLfloat& s, GLfloat& t);

        glm::vec2 texCoord(GLuint idx);
    };

    typedef std::shared_ptr<TexCoords> TexCoordsPtr;
    
};