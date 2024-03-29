#include <ivf/line_trace.h>

#include <ivf/light_manager.h>

using namespace ivf;

LineTrace::LineTrace(int numVertices) : MeshNode(), m_useColor(true), m_color{1.0, 1.0, 1.0, 1.0}
{
    m_numVertices = numVertices;
    this->newMesh(numVertices);
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

void ivf::LineTrace::add(glm::vec3 &vertex)
{
    m_vertices.push_back(vertex);

    for (auto i = 0; i < m_vertices.size(); i++)
    {
        if (i < mesh()->vertices()->size())
        {
			auto v = m_vertices[i];
			mesh()->vertices()->setVertex(i, v.x, v.y, v.z);
		}
    }
}

void ivf::LineTrace::clear()
{
    m_vertices.clear();
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
    /*

    for (auto i = 0; i <= m_xDivisions; i++) {
        GLfloat x = -m_width / 2.0 + m_width * (GLfloat)i / (GLfloat)m_xDivisions;
        if (m_align == GridAlign::XY) {
            if (m_useColor)
                mesh()->color3f(m_color[0], m_color[1], m_color[2]);
            mesh()->normal3d(0.0, 0.0, 1.0);
            mesh()->vertex3d(x, -m_height / 2.0, 0.0);
            if (m_useColor)
                mesh()->color3f(m_color[0], m_color[1], m_color[2]);
            mesh()->normal3d(0.0, 0.0, 1.0);
            mesh()->vertex3d(x, m_height / 2.0, 0.0);
        }
        else if (m_align == GridAlign::XZ) {
            if (m_useColor)
                mesh()->color3f(m_color[0], m_color[1], m_color[2]);
            mesh()->normal3d(0.0, 1.0, 0.0);
            mesh()->vertex3d(x, 0.0, -m_height / 2.0);
            if (m_useColor)
                mesh()->color3f(m_color[0], m_color[1], m_color[2]);
            mesh()->normal3d(0.0, 1.0, 0.0);
            mesh()->vertex3d(x, 0.0, m_height / 2.0);
        }
        else if (m_align == GridAlign::YZ) {
            if (m_useColor)
                mesh()->color3f(m_color[0], m_color[1], m_color[2]);
            mesh()->normal3d(1.0, 0.0, 0.0);
            mesh()->vertex3d(0.0, x, -m_height / 2.0);
            if (m_useColor)
                mesh()->color3f(m_color[0], m_color[1], m_color[2]);
            mesh()->normal3d(1.0, 0.0, 0.0);
            mesh()->vertex3d(0.0, x, m_height / 2.0);
        }
    }

    for (auto i = 0; i <= m_yDivisions; i++) {
        GLfloat y = -m_height / 2.0 + m_height * (GLfloat)i / (GLfloat)m_yDivisions;
        if (m_align == GridAlign::XY) {
            if (m_useColor)
                mesh()->color3f(m_color[0], m_color[1], m_color[2]);
            mesh()->normal3d(0.0, 0.0, 1.0);
            mesh()->vertex3d(-m_width / 2.0, y, 0.0);
            if (m_useColor)
                mesh()->color3f(m_color[0], m_color[1], m_color[2]);
            mesh()->normal3d(0.0, 0.0, 1.0);
            mesh()->vertex3d(m_width / 2.0, y, 0.0);
        }
        else if (m_align == GridAlign::XZ) {
            if (m_useColor)
                mesh()->color3f(m_color[0], m_color[1], m_color[2]);
            mesh()->normal3d(0.0, 1.0, 0.0);
            mesh()->vertex3d(-m_width / 2.0, 0.0, y);
            if (m_useColor)
                mesh()->color3f(m_color[0], m_color[1], m_color[2]);
            mesh()->normal3d(0.0, 1.0, 0.0);
            mesh()->vertex3d(m_width / 2.0, 0.0, y);
        }
        else if (m_align == GridAlign::YZ) {
            if (m_useColor)
                mesh()->color3f(m_color[0], m_color[1], m_color[2]);
            mesh()->normal3d(1.0, 0.0, 0.0);
            mesh()->vertex3d(0.0, -m_width / 2.0, y);
            if (m_useColor)
                mesh()->color3f(m_color[0], m_color[1], m_color[2]);
            mesh()->normal3d(1.0, 0.0, 0.0);
            mesh()->vertex3d(0.0, m_width / 2.0, y);
        }
    }
    */
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
