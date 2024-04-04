#include <ivf/line_trace.h>

#include <ivf/light_manager.h>

using namespace ivf;

LineTrace::LineTrace(int numVertices)
    : MeshNode(), m_useColor(true), m_color{1.0, 1.0, 1.0, 1.0}, m_numVertices(numVertices), m_firstAdd(true)
{
    this->doSetup();
}

LineTrace::~LineTrace()
{}

std::shared_ptr<LineTrace> ivf::LineTrace::create(int numVertices)
{
    return std::make_shared<LineTrace>(numVertices);
}

void ivf::LineTrace::setColor(GLfloat r, GLfloat g, GLfloat b, GLfloat a)
{
    m_color[0] = r;
    m_color[1] = g;
    m_color[2] = b;
    m_color[3] = a;
    this->refresh();
}

void ivf::LineTrace::setUseColor(bool value)
{
    m_useColor = value;
    this->refresh();
}

bool ivf::LineTrace::useColor() const
{
    return m_useColor;
}

void ivf::LineTrace::getColor(GLfloat &r, GLfloat &g, GLfloat &b, GLfloat &a)
{
    r = m_color[0];
    g = m_color[1];
    b = m_color[2];
    a = m_color[3];
}

void ivf::LineTrace::setNumVertices(int numVertices)
{
    m_numVertices = numVertices;
    this->refresh();
}

int ivf::LineTrace::numVertices() const
{
    return m_numVertices;
}

void ivf::LineTrace::setVertex(int idx, glm::vec3 &vertex)
{
    if (idx < mesh()->vertices()->size())
        mesh()->vertices()->setVertex(idx, vertex.x, vertex.y, vertex.z);
}

void ivf::LineTrace::reset()
{
    m_firstAdd = true;
}

void ivf::LineTrace::start(glm::vec3 &vertex)
{
    for (auto i = 0; i < m_numVertices; i++)
        mesh()->vertices()->setVertex(i, vertex.x, vertex.y, vertex.z);

    this->refresh();
}

void ivf::LineTrace::add(glm::vec3 &vertex)
{
    if (m_firstAdd)
    {
        this->start(vertex);
        m_firstAdd = false;
        return;
    }
    //                                                       i
    //                                                       |
    // o --- o --- o --- o --- o --- o --- o --- o --- o --- o
    // 0     1     2     3     4     5     6     7     8     9
    //       0     1     2     3     4     5     6     7     8     9

    for (auto i = m_numVertices - 1; i > 0; i--)
    {
        glm::vec3 v = mesh()->vertices()->vertex(i - 1);
        mesh()->vertices()->setVertex(i, v.x, v.y, v.z);
    }
    mesh()->vertices()->setVertex(0, vertex.x, vertex.y, vertex.z);
}

void ivf::LineTrace::clear()
{}

void ivf::LineTrace::refresh()
{
    this->mesh()->updateVertices();
}

void ivf::LineTrace::doSetup()
{
    this->clear();
    this->newMesh(m_numVertices);

    mesh()->begin(GL_LINE_STRIP);
    for (auto i = 0; i < m_numVertices; i++)
    {
        GLfloat x = 0.0;
        GLfloat y = 0.0;
        GLfloat z = 0.0;
        if (m_useColor)
            mesh()->color3f(m_color[0], m_color[1], m_color[2]);
        mesh()->normal3d(0.0, 0.0, 1.0);
        mesh()->vertex3d(x, y, z);
    }
    mesh()->end();
}

void LineTrace::doPreDraw()
{
    LightManager::instance()->saveState();
    LightManager::instance()->disableLighting();
}

void LineTrace::doPostDraw()
{
    LightManager::instance()->restoreState();
}
