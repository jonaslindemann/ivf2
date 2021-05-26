#include <ivf/meshnode.h>

#include <generator/generator.hpp>
#include <generator/utils.hpp>

using namespace ivf;
using namespace generator;

MeshNode::MeshNode()
{
}

std::shared_ptr<MeshNode> ivf::MeshNode::create()
{
	return std::make_shared<MeshNode>();
}

void MeshNode::addMesh(std::shared_ptr<Mesh> mesh)
{
	m_meshes.push_back(mesh);
}

std::vector<std::shared_ptr<Mesh>> ivf::MeshNode::meshes()
{
	return m_meshes;
}

void MeshNode::newMesh(int vsize, int isize)
{
	this->addMesh(std::make_shared<Mesh>(vsize, isize));
}

std::shared_ptr<Mesh> ivf::MeshNode::mesh(int idx) 
{
	if ((idx >= 0) && (idx < m_meshes.size()))
		return m_meshes[idx];
	else
		return nullptr;
}

void ivf::MeshNode::clear()
{
	m_meshes.clear();
}

void ivf::MeshNode::createFromGenerator(generator::AnyGenerator<generator::MeshVertex>& vertices, generator::AnyGenerator<generator::Triangle>& triangles)
{
    GLuint nVertices = count(vertices);
    GLuint nTriangles = count(triangles);

    this->clear();
    this->newMesh(nVertices, nTriangles);

    mesh()->setGenerateNormals(false);

    mesh()->begin(GL_TRIANGLES);

    while (!vertices.done())
    {
        MeshVertex vertex = vertices.generate();

        mesh()->vertex3f(GLfloat(vertex.position[0]), GLfloat(vertex.position[2]), GLfloat(vertex.position[1]));
        mesh()->normal3f(GLfloat(vertex.normal[0]), GLfloat(vertex.normal[2]), GLfloat(vertex.normal[1]));

        vertices.next();
    }

    while (!triangles.done())
    {
        Triangle triangle = triangles.generate();

        mesh()->index3i(triangle.vertices[0], triangle.vertices[1], triangle.vertices[2]);

        triangles.next();
    }

    mesh()->end();
}

void MeshNode::refresh()
{
	this->doSetup();
}

void ivf::MeshNode::doDraw()
{
	for (auto mesh : m_meshes)
		mesh->draw();
}

void MeshNode::doSetup()
{
}
