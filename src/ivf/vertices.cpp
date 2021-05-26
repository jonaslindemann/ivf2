#include <ivf/vertices.h>

using namespace ivf;

Vertices::Vertices(GLuint nVertices)
:FloatField(nVertices, 3)
{
}

std::shared_ptr<Vertices> ivf::Vertices::create(GLuint nVertices)
{
	return std::make_shared<Vertices>(nVertices);
}

void Vertices::setVertex(GLuint idx, GLfloat x, GLfloat y, GLfloat z)
{
    this->set(idx, 0, x);
    this->set(idx, 1, y);
    this->set(idx, 2, z);
}

void Vertices::getVertex(GLuint idx, GLfloat& x, GLfloat& y, GLfloat& z)
{
	x = this->at(idx, 0);
	y = this->at(idx, 1);
	z = this->at(idx, 2);
}

glm::vec3 ivf::Vertices::vertex(GLuint idx)
{
	return glm::vec3(this->at(idx, 0), this->at(idx, 1), this->at(idx, 2));
}


