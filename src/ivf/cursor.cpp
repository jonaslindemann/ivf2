#include <ivf/cursor.h>

#include <ivf/light_manager.h>
#include <algorithm>
#include <cmath>

using namespace ivf;

Cursor::Cursor(GLfloat size, GLfloat gap, bool showGroundProjection)
    : MeshNode(), m_size(size), m_gap(std::max(0.0f, gap)), m_showGroundProjection(showGroundProjection),
      m_useCustomColors(false)
{
    // Set default colors
    // X-axis: Red
    m_xAxisColor[0] = 1.0f;
    m_xAxisColor[1] = 0.0f;
    m_xAxisColor[2] = 0.0f;
    m_xAxisColor[3] = 1.0f;
    // Y-axis: Green
    m_yAxisColor[0] = 0.0f;
    m_yAxisColor[1] = 1.0f;
    m_yAxisColor[2] = 0.0f;
    m_yAxisColor[3] = 1.0f;
    // Z-axis: Blue
    m_zAxisColor[0] = 0.0f;
    m_zAxisColor[1] = 0.0f;
    m_zAxisColor[2] = 1.0f;
    m_zAxisColor[3] = 1.0f;
    // Ground projection: Gray
    m_groundLineColor[0] = 0.5f;
    m_groundLineColor[1] = 0.5f;
    m_groundLineColor[2] = 0.5f;
    m_groundLineColor[3] = 1.0f;

    this->doSetup();
    this->setName("Cursor");
}

Cursor::~Cursor()
{}

std::shared_ptr<Cursor> Cursor::create(GLfloat size, GLfloat gap, bool showGroundProjection)
{
    return std::make_shared<Cursor>(size, gap, showGroundProjection);
}

void Cursor::setSize(GLfloat size)
{
    m_size = std::max(0.0f, size);
    this->refresh();
}

GLfloat Cursor::size() const
{
    return m_size;
}

void Cursor::setGap(GLfloat gap)
{
    m_gap = std::max(0.0f, gap);
    this->refresh();
}

GLfloat Cursor::gap() const
{
    return m_gap;
}

void Cursor::setShowGroundProjection(bool show)
{
    m_showGroundProjection = show;
    this->refresh();
}

bool Cursor::showGroundProjection() const
{
    return m_showGroundProjection;
}

void Cursor::setAxisColors(const GLfloat xColor[4], const GLfloat yColor[4], const GLfloat zColor[4],
                           const GLfloat groundColor[4])
{
    if (xColor)
    {
        for (int i = 0; i < 4; ++i)
        {
            m_xAxisColor[i] = xColor[i];
        }
    }

    if (yColor)
    {
        for (int i = 0; i < 4; ++i)
        {
            m_yAxisColor[i] = yColor[i];
        }
    }

    if (zColor)
    {
        for (int i = 0; i < 4; ++i)
        {
            m_zAxisColor[i] = zColor[i];
        }
    }

    if (groundColor)
    {
        for (int i = 0; i < 4; ++i)
        {
            m_groundLineColor[i] = groundColor[i];
        }
    }

    m_useCustomColors = true;
    this->refresh();
}

void Cursor::resetToDefaultColors()
{
    // Reset to default colors
    // X-axis: Red
    m_xAxisColor[0] = 1.0f;
    m_xAxisColor[1] = 0.0f;
    m_xAxisColor[2] = 0.0f;
    m_xAxisColor[3] = 1.0f;
    // Y-axis: Green
    m_yAxisColor[0] = 0.0f;
    m_yAxisColor[1] = 1.0f;
    m_yAxisColor[2] = 0.0f;
    m_yAxisColor[3] = 1.0f;
    // Z-axis: Blue
    m_zAxisColor[0] = 0.0f;
    m_zAxisColor[1] = 0.0f;
    m_zAxisColor[2] = 1.0f;
    m_zAxisColor[3] = 1.0f;
    // Ground projection: Gray
    m_groundLineColor[0] = 0.5f;
    m_groundLineColor[1] = 0.5f;
    m_groundLineColor[2] = 0.5f;
    m_groundLineColor[3] = 1.0f;

    this->refresh();
}

void Cursor::setUseCustomColors(bool useCustom)
{
    m_useCustomColors = useCustom;
    this->refresh();
}

bool Cursor::useCustomColors() const
{
    return m_useCustomColors;
}

void Cursor::doSetup()
{
    this->clear();

    // Calculate the number of vertices needed
    // Each axis line needs 2 vertices, ground projection needs 2 vertices
    int vertexCount = 6; // 3 axes * 2 vertices each
    if (m_showGroundProjection)
    {
        vertexCount += 2; // Ground projection line
    }

    this->newMesh(vertexCount, 0, GL_LINES);

    mesh()->begin(GL_LINES);

    // X-axis line (positive direction)
    mesh()->vertex3d(-m_size, 0.0, 0.0);
    if (m_useCustomColors)
    {
        mesh()->color4f(m_xAxisColor[0], m_xAxisColor[1], m_xAxisColor[2], m_xAxisColor[3]);
    }
    else
    {
        mesh()->color3f(1.0, 0.0, 0.0); // Red
    }

    mesh()->vertex3d(m_size, 0.0, 0.0);
    if (m_useCustomColors)
    {
        mesh()->color4f(m_xAxisColor[0], m_xAxisColor[1], m_xAxisColor[2], m_xAxisColor[3]);
    }
    else
    {
        mesh()->color3f(1.0, 0.0, 0.0); // Red
    }

    // Y-axis line (positive direction, with gap consideration for ground projection)
    mesh()->vertex3d(0.0, -m_size, 0.0);
    if (m_useCustomColors)
    {
        mesh()->color4f(m_yAxisColor[0], m_yAxisColor[1], m_yAxisColor[2], m_yAxisColor[3]);
    }
    else
    {
        mesh()->color3f(0.0, 1.0, 0.0); // Green
    }

    mesh()->vertex3d(0.0, m_size, 0.0);
    if (m_useCustomColors)
    {
        mesh()->color4f(m_yAxisColor[0], m_yAxisColor[1], m_yAxisColor[2], m_yAxisColor[3]);
    }
    else
    {
        mesh()->color3f(0.0, 1.0, 0.0); // Green
    }

    // Z-axis line (positive direction)
    mesh()->vertex3d(0.0, 0.0, -m_size);
    if (m_useCustomColors)
    {
        mesh()->color4f(m_zAxisColor[0], m_zAxisColor[1], m_zAxisColor[2], m_zAxisColor[3]);
    }
    else
    {
        mesh()->color3f(0.0, 0.0, 1.0); // Blue
    }

    mesh()->vertex3d(0.0, 0.0, m_size);
    if (m_useCustomColors)
    {
        mesh()->color4f(m_zAxisColor[0], m_zAxisColor[1], m_zAxisColor[2], m_zAxisColor[3]);
    }
    else
    {
        mesh()->color3f(0.0, 0.0, 1.0); // Blue
    }

    // Ground projection line (extends down to XZ plane with gap)
    if (m_showGroundProjection)
    {
        // Start from below the center, with the specified gap

        glm::vec3 p0 = this->pos();

        mesh()->vertex3d(p0.x, p0.y - m_size - m_gap, p0.z);
        if (m_useCustomColors)
        {
            mesh()->color4f(m_groundLineColor[0], m_groundLineColor[1], m_groundLineColor[2], m_groundLineColor[3]);
        }
        else
        {
            mesh()->color3f(0.5, 0.5, 0.5); // Gray
        }

        // Extend down to indicate ground level (we'll use a reasonable length)
        // This could be made configurable in the future
        GLfloat groundProjectionLength = m_size * 2.0f; // Twice the cursor size
        mesh()->vertex3d(p0.x, 0.0, p0.z);
        if (m_useCustomColors)
        {
            mesh()->color4f(m_groundLineColor[0], m_groundLineColor[1], m_groundLineColor[2], m_groundLineColor[3]);
        }
        else
        {
            mesh()->color3f(0.5, 0.5, 0.5); // Gray
        }
    }

    mesh()->end();
}

void Cursor::doPreDraw()
{
    MeshNode::doPreDraw();
    // Disable lighting for cursor rendering to ensure colors are visible
    LightManager::instance()->saveState();
    LightManager::instance()->disableLighting();
}

void Cursor::doPostDraw()
{
    // Restore lighting state
    LightManager::instance()->restoreState();
    MeshNode::doPostDraw();
}
