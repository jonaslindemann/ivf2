#include <ivf/mesh.h>

#include <memory>
#include <iostream>

#include <glad/glad.h>

#include <ivf/shader_manager.h>
#include <ivf/transform_manager.h>
#include <ivf/utils.h>

using namespace ivf;
using namespace std;

glm::vec3 computeNormal(
	glm::vec3 const& a,
	glm::vec3 const& b,
	glm::vec3 const& c)
{
	return glm::normalize(glm::cross(c - a, b - a));
}

Mesh::Mesh(GLuint vsize, GLuint isize, GLuint primType)
	:m_position(0.0f),
	 m_generateNormals(true)
{
	m_primType = primType;
	this->setSize(vsize, isize);
}

Mesh::~Mesh()
{
}

std::shared_ptr<Mesh> ivf::Mesh::create(GLuint vsize, GLuint isize)
{
	return std::make_shared<Mesh>(vsize, isize);
}

void Mesh::setupPrim()
{
	if (m_primType == GL_TRIANGLES)
		m_indices = std::make_shared<Indices>(m_indexSize, 3);
	//if (m_primType == GL_QUADS)
	//	m_indices = std::make_shared<Indices>(new Indices(m_indexSize, 4));
	if (m_primType == GL_LINES)
		m_indices = std::make_shared<Indices>(m_indexSize, 2);
	if (m_primType == GL_LINE_STRIP)
		m_indices = std::make_shared<Indices>(m_indexSize, 1);
	if (m_primType == GL_LINE_LOOP)
		m_indices = std::make_shared<Indices>(m_indexSize, 1);
	if (m_primType == GL_TRIANGLE_STRIP)
		m_indices = std::make_shared<Indices>(m_indexSize, 1);
	if (m_primType == GL_TRIANGLE_FAN)
		m_indices = std::make_shared<Indices>(m_indexSize, 1);
	//if (m_primType == GL_QUAD_STRIP)
	//	m_indices = std::make_unique<Indices>(new Indices(m_indexSize, 1));
	//if (m_primType == GL_POLYGON)
	//	m_indices = std::make_unique<Indices>(new Indices(m_indexSize, 1));
	if (m_primType == GL_POINTS)
		m_indices = std::make_shared<Indices>(m_indexSize, 1);
}


void Mesh::setSize(GLuint vsize, GLuint isize)
{
	m_verts = std::make_shared<Vertices>(vsize);
	m_colors = std::make_shared<Colors>(vsize);
	m_normals = std::make_shared<Normals>(vsize);
	m_texCoords = std::make_shared<TexCoords>(vsize);

	m_glVerts = nullptr;
	m_glColors = nullptr;
	m_glNormals = nullptr;

	m_indexSize = isize;

	if (m_indexSize != 0)
		this->setupPrim();
	else
		m_indices = nullptr;

	m_vertPos = 0;
	m_colorPos = 0;
	m_indexPos = 0;
	m_normalPos = 0;
	m_texCoordPos = 0;

	m_vertexAttrId = -1; 
	m_colorAttrId = -1;
	m_texCoordAttrId = -1;

	this->setVertexAttrId(ShaderManager::instance()->currentProgram()->attribId("aPos"));
	this->setColorAttrId(ShaderManager::instance()->currentProgram()->attribId("aColor"));
	this->setNormalAttrId(ShaderManager::instance()->currentProgram()->attribId("aNormal"));
	this->setTexCoordAttrId(ShaderManager::instance()->currentProgram()->attribId("aTex"));
}

void ivf::Mesh::setGenerateNormals(bool flag)
{
	m_generateNormals = flag;
}

bool ivf::Mesh::generateNormals()
{
	return m_generateNormals;
}

void Mesh::setVertexAttrId(GLuint id)
{
    m_vertexAttrId = id;
}

void Mesh::setColorAttrId(GLuint id)
{
    m_colorAttrId = id;
}

void Mesh::setNormalAttrId(GLuint id)
{
    m_normalAttrId = id;
}

void ivf::Mesh::setTexCoordAttrId(GLuint id)
{
	m_texCoordAttrId = id;
}

void Mesh::begin(GLuint primType)
{
    m_primType = primType;

	if (m_indexSize != 0)
		this->setupPrim();
}

void Mesh::vertex3f(GLfloat x, GLfloat y, GLfloat z)
{
    m_verts->setVertex(m_vertPos++, x, y, z);
}

void Mesh::vertex3d(GLdouble x, GLdouble y, GLdouble z)
{
    m_verts->setVertex(m_vertPos++, GLfloat(x), GLfloat(y), GLfloat(z));
}

void Mesh::vertex3d(const glm::dvec3& v)
{
	this->vertex3d(v.x, v.y, v.z);
}

void ivf::Mesh::vertex3f(const glm::vec3 v)
{
	this->vertex3f(v.x, v.y, v.z);
}

void ivf::Mesh::tex2f(GLfloat s, GLfloat t)
{
	m_texCoords->setTexCoord(m_texCoordPos++, s, t);
}

void ivf::Mesh::normal3f(GLfloat vx, GLfloat vy, GLfloat vz)
{
	m_normals->setNormal(m_normalPos++, vx, vy, vz);
}

void ivf::Mesh::normal3f(const glm::vec3 v)
{
	this->normal3f(v.x, v.y, v.z);
}

void ivf::Mesh::normal3d(GLdouble vx, GLdouble vy, GLdouble vz)
{
	this->normal3f(GLfloat(vx), GLfloat(vy), GLfloat(vz));
}

void Mesh::vertex2f(GLfloat x, GLfloat y)
{
    m_verts->setVertex(m_vertPos++, x, y, 0.0);
}

void Mesh::vertex2d(GLdouble x, GLdouble y)
{
    m_verts->setVertex(m_vertPos++, GLfloat(x), GLfloat(y), 0.0f);
}

void Mesh::color3f(GLfloat r, GLfloat g, GLfloat b)
{
    m_colors->setColor(m_colorPos++, r, g, b);
}

void Mesh::color4f(GLfloat r, GLfloat g, GLfloat b, GLfloat a)
{
    m_colors->setColor(m_colorPos++, r, g, b, a);
}

void Mesh::index1i(GLuint i0)
{
	if (m_indices != 0)
		m_indices->setIndex(m_indexPos++, i0);
}

void Mesh::index2i(GLuint i0, GLuint i1)
{
	if (m_indices != 0)
		m_indices->setIndex(m_indexPos++, i0, i1);
}

void Mesh::index3i(GLuint i0, GLuint i1, GLuint i2)
{
	if (m_indices != 0)
		m_indices->setIndex(m_indexPos++, i0, i1, i2);
}

void Mesh::index4i(GLuint i0, GLuint i1, GLuint i2, GLuint i3)
{
	if (m_indices != 0)
		m_indices->setIndex(m_indexPos++, i0, i1, i2, i3);
}

void Mesh::triQuad(GLdouble w, GLdouble h, GLdouble offset, GLdouble vx, GLdouble vy, GLdouble vz)
{
	glm::dvec3 ez(glm::dvec3(vx, vy, vz));
	glm::dvec3 ezp(glm::dvec3(vx, 0.0, vz));
	ezp = glm::normalize(ezp);
	glm::dvec3 ex(-ezp.z, 0.0, ezp.x);
	glm::dvec3 ey = glm::cross(ez, ex);

	glm::dvec3 p0 = (-w / 2.0)*ex + (-h / 2.0)*ey + (offset)*ez;
	glm::dvec3 p1 = (w / 2.0)*ex + (-h / 2.0)*ey + (offset)*ez;
	glm::dvec3 p2 = (w / 2.0)*ex + (h / 2.0)*ey + (offset)*ez;

	this->vertex3d(p0);
	this->vertex3d(p1);
	this->vertex3d(p2);

	p0 = (-w / 2.0)*ex + (-h / 2.0)*ey + (offset)*ez;
	p1 = (-w / 2.0)*ex + (h / 2.0)*ey + (offset)*ez;
	p2 = (w / 2.0)*ex + (h / 2.0)*ey + (offset)*ez;

	this->vertex3d(p0);
	this->vertex3d(p1);
	this->vertex3d(p2);
}


void Mesh::end()
{
	// Compute actual gl arrays

	if (m_primType == GL_TRIANGLES)
	{
		int pos = 0;

		glm::vec3 points[3];
		glm::vec3 norm;

		if (m_indices != nullptr)
		{
			if (m_generateNormals)
			{
				for (GLuint i = 0; i < m_indices->rows(); i++)
				{
					points[0] = m_verts->vertex(m_indices->at(i, 0));
					points[1] = m_verts->vertex(m_indices->at(i, 1));
					points[2] = m_verts->vertex(m_indices->at(i, 2));
					norm = computeNormal(points[0], points[1], points[2]);
					m_normals->setNormal(m_indices->at(i, 0), -norm);
					m_normals->setNormal(m_indices->at(i, 1), -norm);
					m_normals->setNormal(m_indices->at(i, 2), -norm);
				}
			}
		}
	}

	GLenum err;
    m_VAO = std::make_unique<VertexArray>();
    m_VAO->bind();

    m_vertexVBO = std::make_unique<VertexBuffer>();
	//m_vertexVBO->setArray(m_glVerts.get());
	m_vertexVBO->setArray(m_verts.get());

	glEnableVertexAttribArray(m_vertexAttrId);
	glVertexAttribPointer(m_vertexAttrId, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

	if (m_indices != 0)
	{
		m_indexVBO = std::make_unique<IndexBuffer>();
		m_indexVBO->setArray(m_indices.get());
	}

	if (m_colorAttrId != -1)
	{
		m_colorVBO = std::make_unique<VertexBuffer>();
		//m_colorVBO->setArray(m_glColors.get());
		m_colorVBO->setArray(m_colors.get());
		glEnableVertexAttribArray(m_colorAttrId);
		glVertexAttribPointer(m_colorAttrId, 4, GL_FLOAT, GL_FALSE, 0, (void*)0);
	}

	if (m_normalAttrId != -1)
	{
		m_normalVBO = std::make_unique<VertexBuffer>();
		//m_normalVBO->setArray(m_glNormals.get());
		m_normalVBO->setArray(m_normals.get());
		glEnableVertexAttribArray(m_normalAttrId);
		glVertexAttribPointer(m_normalAttrId, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
	}

	if (m_texCoordAttrId != -1)
	{
		m_texCoordVBO = std::make_unique<VertexBuffer>();
		m_texCoordVBO->setArray(m_texCoords.get());
		glEnableVertexAttribArray(m_texCoordAttrId);
		glVertexAttribPointer(m_texCoordAttrId, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
	}

	m_VAO->unbind();
 	err = checkPrintError("Mesh", __FILE__, __LINE__);
}

void Mesh::draw()
{
    m_VAO->bind();
	if (m_indices != nullptr)
	{
		GL_ERR(glDrawElements(m_primType, m_indices->size(), GL_UNSIGNED_INT, 0));
	}
	else
	{
		GL_ERR(glDrawArrays(m_primType, 0, m_verts->rows()));
	}
    m_VAO->unbind();
}

void ivf::Mesh::drawAsPrim(GLuint prim)
{
	m_VAO->bind();
	//err = checkPrintError("__FILE__, __LINE__");
	glDrawArrays(prim, 0, m_glVerts->size());
	m_VAO->unbind();
	//err = checkPrintError("__FILE__, __LINE__");
}

void ivf::Mesh::setPos(glm::vec3 pos)
{
	m_position = pos;
}

glm::vec3 ivf::Mesh::pos()
{
	return m_position;
}

GLuint ivf::Mesh::currentIndexPos()
{
	return m_indexPos;
}

std::shared_ptr<Normals> ivf::Mesh::normals()
{
	return m_normals;
}

std::shared_ptr<Vertices> ivf::Mesh::vertices()
{
	return m_verts;
}

std::shared_ptr<Indices> ivf::Mesh::indices()
{
	return m_indices;
}

void ivf::Mesh::print()
{
	if (m_indices != nullptr)
	{
		//GL_ERR(glDrawElements(m_primType, m_indices->size(), GL_UNSIGNED_INT, 0));
	}
	else
	{
		//GL_ERR(glDrawArrays(m_primType, 0, m_verts->rows()));
		cout << "---------" << endl;
		cout << "Vertices:" << endl;
		if (m_verts!=nullptr)
			m_verts->print();
		cout << "Normals:" << endl;
		if (m_normals!=nullptr)
			m_normals->print();
		cout << "Colors:" << endl;
		if (m_colors != nullptr)
			m_colors->print();
	}
}
