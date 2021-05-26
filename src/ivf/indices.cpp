#include <ivf/indices.h>

using namespace ivf;

Indices::Indices(GLuint nIndices, GLuint cols)
:IntField(nIndices, cols)
{
}

void Indices::setIndex(GLuint idx, GLuint i0)
{
	this->set(idx, 0, i0);
}

void Indices::setIndex(GLuint idx, GLuint i0, GLuint i1)
{
	this->set(idx, 0, i0);
	this->set(idx, 1, i1);
}

void Indices::setIndex(GLuint idx, GLuint i0, GLuint i1, GLuint i2)
{
	this->set(idx, 0, i0);
	this->set(idx, 1, i1);
	this->set(idx, 2, i2);
}

void Indices::setIndex(GLuint idx, GLuint i0, GLuint i1, GLuint i2, GLuint i3)
{
	this->set(idx, 0, i0);
	this->set(idx, 1, i1);
	this->set(idx, 2, i2);
	this->set(idx, 3, i3);
}


