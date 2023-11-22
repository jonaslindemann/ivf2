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

void ivf::Indices::index(GLuint idx, GLuint& i0)
{
	i0 = this->at(idx, 0);
}

void ivf::Indices::index(GLuint idx, GLuint& i0, GLuint& i1)
{
	i0 = this->at(idx, 0);
	i1 = this->at(idx, 1);
}

void ivf::Indices::index(GLuint idx, GLuint& i0, GLuint& i1, GLuint& i2)
{
	i0 = this->at(idx, 0);
	i1 = this->at(idx, 1);
	i2 = this->at(idx, 2);
}

void ivf::Indices::index(GLuint idx, GLuint& i0, GLuint& i1, GLuint& i2, GLuint& i3)
{
	i0 = this->at(idx, 0);
	i1 = this->at(idx, 1);
	i2 = this->at(idx, 2);
	i3 = this->at(idx, 3);
}


