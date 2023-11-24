#pragma once

#include <ivf/float_field.h>

namespace ivf {
    
    class Colors : public FloatField {
    public:
        Colors(GLuint nColors);

        static std::shared_ptr<Colors> create(GLuint nColors);
        
        void setColor(GLuint idx, GLfloat r, GLfloat g, GLfloat b, GLfloat a = 1.0);
		void getColor(GLuint idx, GLfloat& r, GLfloat& g, GLfloat& b, GLfloat& a);
	};

    typedef std::shared_ptr<Colors> ColorsPtr;
    
};
