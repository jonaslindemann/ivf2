#include <ivf/line_grid.h>

#include <ivf/light_manager.h>

using namespace ivf;

LineGrid::LineGrid(GLfloat width, GLfloat height, GLint xDiv, GLint yDiv)
    : MeshNode(), m_height(height), m_width(width), m_xDivisions(xDiv), m_yDivisions(yDiv), m_align(GridAlign::XZ), m_useColor(true), m_color{ 1.0, 1.0, 1.0, 1.0 }
{
    this->newMesh((m_xDivisions + 1) * 2 + (m_yDivisions + 1) * 2);
    this->doSetup();
}

LineGrid::~LineGrid()
{
}

std::shared_ptr<LineGrid> ivf::LineGrid::create(GLfloat width, GLfloat height, GLint xDiv, GLint yDiv)
{
    return std::make_shared<LineGrid>(width, height, xDiv, yDiv);
}

void LineGrid::setSize(GLfloat width, GLfloat height)
{
    m_width = width;
    m_height = height;
    this->refresh();
}

void LineGrid::setDivisions(GLint xDivisions, GLint yDivisions)
{
    m_xDivisions = xDivisions;
    m_yDivisions = yDivisions;
    this->newMesh((m_xDivisions + 1) * 2 + (m_yDivisions + 1) * 2);
    this->refresh();
}

void ivf::LineGrid::setColor(GLfloat r, GLfloat g, GLfloat b, GLfloat a)
{
    m_color[0] = r;
    m_color[1] = g;
    m_color[2] = b;
    m_color[3] = a;
    this->refresh();
}

void ivf::LineGrid::setUseColor(bool value)
{
    m_useColor = value;
    this->refresh();
}

bool ivf::LineGrid::useColor() const
{
    return m_useColor;
}

void ivf::LineGrid::getColor(GLfloat &r, GLfloat &g, GLfloat &b, GLfloat &a)
{
    r = m_color[0];
	g = m_color[1];
	b = m_color[2];
	a = m_color[3];
}

GLfloat LineGrid::width() const
{
    return m_width;
}

GLfloat LineGrid::height() const
{
    return m_height;
}

GLint LineGrid::xDivisions() const
{
    return m_xDivisions;
}

GLint LineGrid::yDivisions() const
{
    return m_yDivisions;
}

void ivf::LineGrid::setAlign(GridAlign align)
{
    m_align = align;
    this->refresh();
}

GridAlign ivf::LineGrid::align() const
{
    return m_align;
}

void ivf::LineGrid::doSetup()
{
    this->clear();
    this->newMesh((m_xDivisions + 1) * 2 + (m_yDivisions + 1) * 2);

    mesh()->begin(GL_LINES);

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

    mesh()->end();
}

void LineGrid::doPreDraw()
{
    LightManager::instance()->saveState();
    LightManager::instance()->disableLighting();
}

void LineGrid::doPostDraw()
{
    LightManager::instance()->restoreState();
}
