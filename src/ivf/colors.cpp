#include <ivf/colors.h>

using namespace ivf;

Colors::Colors(GLuint nColors)
:FloatField(nColors, 4)
{
}

void Colors::setColor(GLuint idx, GLfloat r, GLfloat g, GLfloat b, GLfloat a)
{
    this->set(idx, 0, r);
    this->set(idx, 1, g);
    this->set(idx, 2, b);
    this->set(idx, 3, a);
}

void Colors::getColor(GLuint idx, GLfloat& r, GLfloat& g, GLfloat& b, GLfloat& a)
{
	r = this->at(idx, 0);
	g = this->at(idx, 1);
	b = this->at(idx, 2);
	a = this->at(idx, 3);
}

std::shared_ptr<Colors> ivf::Colors::create(GLuint nColors)
{
    return std::make_shared<Colors>(nColors);
}
