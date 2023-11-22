#include <ivf/meshnode.h>

#include <generator/generator.hpp>
#include <generator/utils.hpp>

#include <iostream>

using namespace std;
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

void MeshNode::newMesh(int vsize, int isize, GLuint primType)
{
	this->addMesh(std::make_shared<Mesh>(vsize, isize, primType));
}

std::shared_ptr<Mesh> ivf::MeshNode::mesh(int idx) 
{
	if ((idx >= 0) && (idx < m_meshes.size()))
		return m_meshes[idx];
	else
		return nullptr;
}

std::shared_ptr<Mesh> ivf::MeshNode::lastMesh()
{
    if (m_meshes.size()!=0)
        return m_meshes[m_meshes.size()-1];
    else
        return nullptr;
}

std::shared_ptr<Mesh> ivf::MeshNode::currentMesh()
{
    return lastMesh();
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
        mesh()->tex2f(vertex.texCoord[0], vertex.texCoord[1]);

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

void ivf::MeshNode::debugFromGenerator(generator::AnyGenerator<generator::MeshVertex>& vertices, generator::AnyGenerator<generator::Triangle>& triangles)
{
    GLuint nVertices = count(vertices);
    GLuint nTriangles = count(triangles);

    while (!vertices.done())
    {
        MeshVertex vertex = vertices.generate();

        cout << "v: (" << vertex.position[0] << ", " << vertex.position[1] << ", " << vertex.position[2] << ")" << endl;
        cout << "n: (" << vertex.normal[0] << ", " << vertex.normal[1] << ", " << vertex.normal[2] << ")" << endl;
        cout << "t: (" << vertex.texCoord[0] << ", " << vertex.texCoord[1] << ")" << endl;

        vertices.next();
    }

    while (!triangles.done())
    {
        Triangle triangle = triangles.generate();

        cout << "i: (" << triangle.vertices[0] << ", " << triangle.vertices[1] << ", " << triangle.vertices[2] << endl;

        triangles.next();
    }
}

void MeshNode::refresh()
{
	this->doSetup();
}

void ivf::MeshNode::print()
{
    for (auto& mesh : m_meshes)
        mesh->print();
}

void ivf::MeshNode::doDraw()
{
	for (auto mesh : m_meshes)
		mesh->draw();
}

void MeshNode::doSetup()
{
}
