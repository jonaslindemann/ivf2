#include <ivf/mesh_node.h>

#include <ivf/mesh_manager.h>
#include <ivf/light_manager.h>

#include <generator/generator.hpp>
#include <generator/utils.hpp>

#include <iostream>

using namespace std;
using namespace ivf;
using namespace generator;

MeshNode::MeshNode() : TransformNode() // Initialize the base class TransformNode
{}

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

void MeshNode::newMesh(int vsize, int isize, GLuint primType, GLenum usage)
{
    this->addMesh(std::make_shared<Mesh>(vsize, isize, primType, usage));
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
    if (m_meshes.size() != 0)
        return m_meshes[m_meshes.size() - 1];
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

void ivf::MeshNode::setWireframe(bool flag)
{
    for (auto &mesh : m_meshes)
    {
        mesh->setWireframe(flag);
    }
}

void ivf::MeshNode::createFromGenerator(generator::AnyGenerator<generator::MeshVertex> &vertices,
                                        generator::AnyGenerator<generator::Triangle> &triangles)
{
    GLuint nVertices = count(vertices);
    GLuint nTriangles = count(triangles);

    this->clear();
    this->newMesh(nVertices, nTriangles, GL_TRIANGLES, ivf::mmDefaultMeshUsage());

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

void ivf::MeshNode::debugFromGenerator(generator::AnyGenerator<generator::MeshVertex> &vertices,
                                       generator::AnyGenerator<generator::Triangle> &triangles)
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

void ivf::MeshNode::updateVertices()
{
    for (auto &mesh : m_meshes)
    {
        mesh->updateVertices();
    }
}

void ivf::MeshNode::updateNormals()
{
    for (auto &mesh : m_meshes)
    {
        mesh->updateNormals();
    }
}

void ivf::MeshNode::print()
{
    for (auto &mesh : m_meshes)
        mesh->print();
}

void ivf::MeshNode::setShowNormals(bool show, float length)
{
    m_showNormals = show;
    m_normalLength = length;
    if (show)
    {
        updateNormalVisualization();
    }
}

void ivf::MeshNode::updateNormalVisualization()
{
    if (!m_showNormals || m_meshes.empty())
        return;

    // Create combined normal visualization for all meshes
    int totalVertices = 0;
    for (const auto &mesh : m_meshes)
    {
        if (mesh->vertices())
        {
            totalVertices += mesh->vertices()->rows();
        }
    }

    m_normalVisMesh = std::make_shared<Mesh>(totalVertices * 2, 0, GL_LINES);
    m_normalVisMesh->begin(GL_LINES);

    for (const auto &mesh : m_meshes)
    {
        if (!mesh->vertices() || !mesh->normals())
            continue;

        for (int i = 0; i < mesh->vertices()->rows(); ++i)
        {
            glm::vec3 vertex = mesh->vertices()->vertex(i);
            glm::vec3 normal = mesh->normals()->normal(i);

            m_normalVisMesh->vertex3f(vertex);
            m_normalVisMesh->color3f(1.0f, 1.0f, 0.0f); // Yellow

            m_normalVisMesh->vertex3f(vertex + normal * m_normalLength);
            m_normalVisMesh->color3f(0.0f, 1.0f, 1.0f); // Cyan
        }
    }

    m_normalVisMesh->end();
}

bool ivf::MeshNode::showNormals() const
{
    return false;
}

void ivf::MeshNode::doDraw()
{
    for (auto &mesh : m_meshes)
        if (mesh->enabled())
            mesh->draw();

    if (m_showNormals && m_normalVisMesh)
    {
        ivf::LightManager::instance()->setUseLighting(false); // Disable lighting for normal visualization
        m_normalVisMesh->draw();
        ivf::LightManager::instance()->setUseLighting(true); // Disable lighting for normal visualization
    }
}

void MeshNode::doSetup()
{}
