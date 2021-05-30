#include <ivf/texcoords.h>

using namespace ivf;

TexCoords::TexCoords(GLuint nVertices)
:FloatField(nVertices, 2)
{
}

std::shared_ptr<TexCoords> TexCoords::create(GLuint nVertices)
{
	return std::make_shared<TexCoords>(nVertices);
}

void TexCoords::setTexCoord(GLuint idx, GLfloat s, GLfloat t)
{
    this->set(idx, 0, s);
    this->set(idx, 1, t);
}

void TexCoords::getTexCoord(GLuint idx, GLfloat& s, GLfloat& t)
{
	s = this->at(idx, 0);
	t = this->at(idx, 1);
}

glm::vec2 TexCoords::texCoord(GLuint idx)
{
	return glm::vec2(this->at(idx, 0), this->at(idx, 1));
}


