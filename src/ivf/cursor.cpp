#include <ivf/cursor.h>

#include <ivf/light_manager.h>
#include <algorithm>
#include <cmath>

using namespace ivf;

Cursor::Cursor(GLfloat size, GLfloat gap, bool showGroundProjection)
    : MeshNode(), m_size(size), m_gap(std::max(0.0f, gap)), m_showGroundProjection(showGroundProjection),
      m_useCustomColors(false), m_groundProjectionStartIdx(-1), m_groundProjectionEndIdx(-1), m_needsFullRebuild(true)
{
    m_xAxisColor[0] = 0.8f;
    m_xAxisColor[1] = 0.8f;
    m_xAxisColor[2] = 0.8f;
    m_xAxisColor[3] = 1.0f;
    m_yAxisColor[0] = 0.8f;
    m_yAxisColor[1] = 0.8f;
    m_yAxisColor[2] = 0.8f;
    m_yAxisColor[3] = 1.0f;
    m_zAxisColor[0] = 0.8f;
    m_zAxisColor[1] = 0.8f;
    m_zAxisColor[2] = 0.8f;
    m_zAxisColor[3] = 1.0f;
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
    if (m_size != size)
    {
        m_size = std::max(0.0f, size);
        m_needsFullRebuild = true;
        this->refresh();
    }
}

GLfloat Cursor::size() const
{
    return m_size;
}

void Cursor::setGap(GLfloat gap)
{
    GLfloat newGap = std::max(0.0f, gap);
    if (m_gap != newGap)
    {
        m_gap = newGap;
        if (m_showGroundProjection)
        {
            updateGroundProjectionVertices();
        }
    }
}

GLfloat Cursor::gap() const
{
    return m_gap;
}

void Cursor::setShowGroundProjection(bool show)
{
    if (m_showGroundProjection != show)
    {
        m_showGroundProjection = show;
        m_needsFullRebuild = true;
        this->refresh();
    }
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
            m_xAxisColor[i] = xColor[i];
    }

    if (yColor)
    {
        for (int i = 0; i < 4; ++i)
            m_yAxisColor[i] = yColor[i];
    }

    if (zColor)
    {
        for (int i = 0; i < 4; ++i)
            m_zAxisColor[i] = zColor[i];
    }

    if (groundColor)
    {
        for (int i = 0; i < 4; ++i)
            m_groundLineColor[i] = groundColor[i];
    }

    m_useCustomColors = true;
    m_needsFullRebuild = true;
    this->refresh();
}

void Cursor::resetToDefaultColors()
{
    // Reset to default colors
    // X-axis: Red
    m_xAxisColor[0] = 0.8f;
    m_xAxisColor[1] = 0.8f;
    m_xAxisColor[2] = 0.8f;
    m_xAxisColor[3] = 1.0f;
    // Y-axis: Green
    m_yAxisColor[0] = 0.8f;
    m_yAxisColor[1] = 0.8f;
    m_yAxisColor[2] = 0.8f;
    m_yAxisColor[3] = 1.0f;
    // Z-axis: Blue
    m_zAxisColor[0] = 0.8f;
    m_zAxisColor[1] = 0.8f;
    m_zAxisColor[2] = 0.8f;
    m_zAxisColor[3] = 1.0f;
    // Ground projection: Gray
    m_groundLineColor[0] = 0.5f;
    m_groundLineColor[1] = 0.5f;
    m_groundLineColor[2] = 0.5f;
    m_groundLineColor[3] = 1.0f;

    m_needsFullRebuild = true;
    this->refresh();
}

void Cursor::setUseCustomColors(bool useCustom)
{
    if (m_useCustomColors != useCustom)
    {
        m_useCustomColors = useCustom;
        m_needsFullRebuild = true;
        this->refresh();
    }
}

bool Cursor::useCustomColors() const
{
    return m_useCustomColors;
}

void Cursor::updatePosition(const glm::vec3 &position)
{
    // Update the cursor's world position
    this->setPos(position);

    // Only update ground projection vertices if they exist
    if (m_showGroundProjection && m_groundProjectionStartIdx >= 0 && m_groundProjectionEndIdx >= 0)
    {
        updateGroundProjectionVertices();
    }
}

void Cursor::updateGroundProjectionVertices()
{
    if (!m_showGroundProjection || m_groundProjectionStartIdx < 0 || m_groundProjectionEndIdx < 0)
        return;

    // Use LOCAL coordinates relative to cursor origin (0,0,0)
    // TransformNode will handle the world positioning
    glm::vec3 cursorPos = this->pos();

    // Update start vertex (relative to cursor center with gap)
    this->mesh()->vertices()->setVertex(m_groundProjectionStartIdx,
                                        0.0f,   // X: stay at cursor center
                                        -m_gap, // Y: gap below cursor
                                        0.0f);  // Z: stay at cursor center

    // Update end vertex (extend down to ground level in local coordinates)
    // Calculate how far down to go from current cursor position
    float groundDistance = cursorPos.y; // Distance from cursor to ground (Y=0)
    this->mesh()->vertices()->setVertex(m_groundProjectionEndIdx,
                                        0.0f,            // X: stay at cursor center
                                        -groundDistance, // Y: extend to ground level
                                        0.0f);           // Z: stay at cursor center

    // Efficiently update only the vertex buffer
    this->mesh()->updateVertices();
}

void Cursor::doSetup()
{
    // Only do full rebuild if needed
    if (!m_needsFullRebuild && mesh() && m_showGroundProjection)
    {
        updateGroundProjectionVertices();
        return;
    }

    this->clear();

    // Calculate the number of vertices needed
    int vertexCount = 6; // 3 axes * 2 vertices each
    if (m_showGroundProjection)
    {
        vertexCount += 2; // Ground projection line

        m_groundProjectionStartIdx = 6; // Start index for ground projection line
        m_groundProjectionEndIdx = 7;   // End index for ground projection line
    }

    this->newMesh(vertexCount, 0, GL_LINES);

    mesh()->begin(GL_LINES);

    // X-axis line
    mesh()->vertex3d(-m_size, 0.0, 0.0);
    if (m_useCustomColors)
        mesh()->color4f(m_xAxisColor[0], m_xAxisColor[1], m_xAxisColor[2], m_xAxisColor[3]);
    else
        mesh()->color3f(0.8, 0.8, 0.8); // Red

    mesh()->vertex3d(m_size, 0.0, 0.0);
    if (m_useCustomColors)
        mesh()->color4f(m_xAxisColor[0], m_xAxisColor[1], m_xAxisColor[2], m_xAxisColor[3]);
    else
        mesh()->color3f(0.8, 0.8, 0.8); // Red

    // Y-axis line
    mesh()->vertex3d(0.0, -m_size, 0.0);
    if (m_useCustomColors)
        mesh()->color4f(m_yAxisColor[0], m_yAxisColor[1], m_yAxisColor[2], m_yAxisColor[3]);
    else
        mesh()->color3f(0.8, 0.8, 0.8); // Green

    mesh()->vertex3d(0.0, m_size, 0.0);
    if (m_useCustomColors)
        mesh()->color4f(m_yAxisColor[0], m_yAxisColor[1], m_yAxisColor[2], m_yAxisColor[3]);
    else
        mesh()->color3f(0.8, 0.8, 0.8); // Green

    // Z-axis line
    mesh()->vertex3d(0.0, 0.0, -m_size);
    if (m_useCustomColors)
        mesh()->color4f(m_zAxisColor[0], m_zAxisColor[1], m_zAxisColor[2], m_zAxisColor[3]);
    else
        mesh()->color3f(0.8, 0.8, 0.8); // Blue

    mesh()->vertex3d(0.0, 0.0, m_size);
    if (m_useCustomColors)
        mesh()->color4f(m_zAxisColor[0], m_zAxisColor[1], m_zAxisColor[2], m_zAxisColor[3]);
    else
        mesh()->color3f(0.8, 0.8, 0.8); // Blue

    if (m_showGroundProjection)
    {
        // Ground projection line (initially set, will be updated in updateGroundProjectionVertices)
        mesh()->vertex3d(0.0f, -m_gap, 0.0f); // Start just below cursor with gap
        if (m_useCustomColors)
            mesh()->color4f(m_groundLineColor[0], m_groundLineColor[1], m_groundLineColor[2], m_groundLineColor[3]);
        else
            mesh()->color3f(0.5, 0.5, 0.5);          // Gray
        mesh()->vertex3d(0.0f, -m_gap - 1.0f, 0.0f); // Extend downwards; actual Y will be set in update
        if (m_useCustomColors)
            mesh()->color4f(m_groundLineColor[0], m_groundLineColor[1], m_groundLineColor[2], m_groundLineColor[3]);
        else
            mesh()->color3f(0.5, 0.5, 0.5); // Gray
    }

    mesh()->end();

    // Ground projection line
    if (m_showGroundProjection)
    {
        updateGroundProjectionVertices();
    }
    else
    {
        // Reset indices when ground projection is disabled
        m_groundProjectionStartIdx = -1;
        m_groundProjectionEndIdx = -1;
    }

    m_needsFullRebuild = false;
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
