#include <ivf/normals.h>

using namespace ivf;

Normals::Normals(GLuint nNormals)
:FloatField(nNormals, 3)
{
}

std::shared_ptr<Normals> ivf::Normals::create(GLuint nNormals)
{
    return std::make_shared<Normals>(nNormals);
}

void Normals::setNormal(GLuint idx, GLfloat nx, GLfloat ny, GLfloat nz)
{
    this->set(idx, 0, nx);
    this->set(idx, 1, ny);
    this->set(idx, 2, nz);
}

void ivf::Normals::getNormal(GLuint idx, GLfloat& nx, GLfloat& ny, GLfloat& nz)
{
    nx = this->get(idx, 0);
    ny = this->get(idx, 1);
    nz = this->get(idx, 2);
}

glm::vec3 ivf::Normals::normal(GLuint idx)
{
    float nx = this->get(idx, 0);
    float ny = this->get(idx, 1);
    float nz = this->get(idx, 2);

    return glm::vec3(nx, ny, nz);
}

void ivf::Normals::setNormal(GLuint idx, glm::vec3 normal)
{
    this->set(idx, 0, normal.x);
    this->set(idx, 1, normal.y);
    this->set(idx, 2, normal.z);
}

