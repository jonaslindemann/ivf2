#include <ivf/grid.h>

#include <ivf/light_manager.h>

#include <iostream>

using namespace std;
using namespace ivf;

Grid::Grid()
    : m_tickX(11), m_tickY(11), m_tickZ(11), m_tickSpacingX(1.0e-0), m_tickSpacingY(1.0e-0), m_tickSpacingZ(1.0e-0),
      m_gridType(GridType::Points), m_tickColor{0.2, 0.2, 0.2, 1.0}, m_lineColor{0.1, 0.1, 0.1, 1.0}, m_markerSize(0.1)

{
    this->doSetup();
}

Grid::~Grid()
{}

std::shared_ptr<Grid> Grid::create()
{
    return std::make_shared<Grid>();
}

void Grid::doSetup()
{
    long x, y, z;

    clear();

    newMesh(m_tickX * m_tickY * m_tickZ);

    GLfloat xSize = m_tickSpacingX * (m_tickX - 1);
    GLfloat ySize = m_tickSpacingY * (m_tickY - 1);
    GLfloat zSize = m_tickSpacingZ * (m_tickZ - 1);

    currentMesh()->begin(GL_POINTS);
    for (x = 0; x < m_tickX; x++)
        for (y = 0; y < m_tickY; y++)
            for (z = 0; z < m_tickZ; z++)
            {
                currentMesh()->vertex3f(-xSize / 2.0f + (float)x * m_tickSpacingX,
                                        -ySize / 2.0f + (float)y * m_tickSpacingY,
                                        -zSize / 2.0f + float(z) * m_tickSpacingZ);

                currentMesh()->color4f(m_tickColor[0], m_tickColor[1], m_tickColor[2], m_tickColor[3]);
            }
    currentMesh()->end();

    newMesh(2 * (m_tickX * m_tickY + m_tickX * m_tickZ + m_tickY * m_tickZ));

    xSize = m_tickSpacingX * (m_tickX - 1);
    ySize = m_tickSpacingY * (m_tickY - 1);
    zSize = m_tickSpacingZ * (m_tickZ - 1);

    currentMesh()->begin(GL_LINES);
    for (x = 0; x < m_tickX; x++)
        for (y = 0; y < m_tickY; y++)
        {
            currentMesh()->vertex3f(-xSize / 2.0f + (float)x * m_tickSpacingX,
                                    -ySize / 2.0f + (float)y * m_tickSpacingY, -zSize / 2.0f);

            currentMesh()->color4f(m_lineColor[0], m_lineColor[1], m_lineColor[2], m_lineColor[3]);

            currentMesh()->vertex3f(-xSize / 2.0f + (float)x * m_tickSpacingX,
                                    -ySize / 2.0f + (float)y * m_tickSpacingY, zSize / 2.0f);

            currentMesh()->color4f(m_lineColor[0], m_lineColor[1], m_lineColor[2], m_lineColor[3]);
        }
    for (x = 0; x < m_tickX; x++)
        for (z = 0; z < m_tickZ; z++)
        {
            currentMesh()->vertex3f(-xSize / 2.0f + (float)x * m_tickSpacingX, -ySize / 2.0f,
                                    -zSize / 2.0f + float(z) * m_tickSpacingZ);
            currentMesh()->color4f(m_lineColor[0], m_lineColor[1], m_lineColor[2], m_lineColor[3]);
            currentMesh()->vertex3f(-xSize / 2.0f + (float)x * m_tickSpacingX, ySize / 2.0f,
                                    -zSize / 2.0f + float(z) * m_tickSpacingZ);
            currentMesh()->color4f(m_lineColor[0], m_lineColor[1], m_lineColor[2], m_lineColor[3]);
        }

    for (y = 0; y < m_tickY; y++)
        for (z = 0; z < m_tickZ; z++)
        {
            currentMesh()->vertex3f(-xSize / 2.0f, -ySize / 2.0f + (float)y * m_tickSpacingY,
                                    -zSize / 2.0f + float(z) * m_tickSpacingZ);
            currentMesh()->color4f(m_lineColor[0], m_lineColor[1], m_lineColor[2], m_lineColor[3]);
            currentMesh()->vertex3f(xSize / 2.0f, -ySize / 2.0f + (float)y * m_tickSpacingY,
                                    -zSize / 2.0f + float(z) * m_tickSpacingZ);
            currentMesh()->color4f(m_lineColor[0], m_lineColor[1], m_lineColor[2], m_lineColor[3]);
        }
    currentMesh()->end();

    newMesh(m_tickX * m_tickY * m_tickZ * 2 * 3);

    xSize = m_tickSpacingX * (m_tickX - 1);
    ySize = m_tickSpacingY * (m_tickY - 1);
    zSize = m_tickSpacingZ * (m_tickZ - 1);

    currentMesh()->begin(GL_LINES);
    for (x = 0; x < m_tickX; x++)
        for (y = 0; y < m_tickY; y++)
            for (z = 0; z < m_tickZ; z++)
            {
                if (x == 0)
                    currentMesh()->vertex3f(-xSize / 2.0f + (float)x * m_tickSpacingX,
                                            -ySize / 2.0f + (float)y * m_tickSpacingY,
                                            -zSize / 2.0f + float(z) * m_tickSpacingZ);
                else
                    currentMesh()->vertex3f(-xSize / 2.0f + (float)x * m_tickSpacingX - m_markerSize / 2.0f,
                                            -ySize / 2.0f + (float)y * m_tickSpacingY,
                                            -zSize / 2.0f + float(z) * m_tickSpacingZ);

                currentMesh()->color4f(m_tickColor[0], m_tickColor[1], m_tickColor[2], m_tickColor[3]);

                if (x == m_tickX - 1)
                    currentMesh()->vertex3f(-xSize / 2.0f + (float)x * m_tickSpacingX,
                                            -ySize / 2.0f + (float)y * m_tickSpacingY,
                                            -zSize / 2.0f + float(z) * m_tickSpacingZ);
                else
                    currentMesh()->vertex3f(-xSize / 2.0f + (float)x * m_tickSpacingX + m_markerSize / 2.0f,
                                            -ySize / 2.0f + (float)y * m_tickSpacingY,
                                            -zSize / 2.0f + float(z) * m_tickSpacingZ);

                currentMesh()->color4f(m_tickColor[0], m_tickColor[1], m_tickColor[2], m_tickColor[3]);

                if (y == 0)
                    currentMesh()->vertex3f(-xSize / 2.0f + (float)x * m_tickSpacingX,
                                            -ySize / 2.0f + (float)y * m_tickSpacingY,
                                            -zSize / 2.0f + float(z) * m_tickSpacingZ);
                else
                    currentMesh()->vertex3f(-xSize / 2.0f + (float)x * m_tickSpacingX,
                                            -ySize / 2.0f + (float)y * m_tickSpacingY - m_markerSize / 2.0f,
                                            -zSize / 2.0f + float(z) * m_tickSpacingZ);

                currentMesh()->color4f(m_tickColor[0], m_tickColor[1], m_tickColor[2], m_tickColor[3]);

                if (y == m_tickY - 1)
                    currentMesh()->vertex3f(-xSize / 2.0f + (float)x * m_tickSpacingX,
                                            -ySize / 2.0f + (float)y * m_tickSpacingY,
                                            -zSize / 2.0f + float(z) * m_tickSpacingZ);
                else
                    currentMesh()->vertex3f(-xSize / 2.0f + (float)x * m_tickSpacingX,
                                            -ySize / 2.0f + (float)y * m_tickSpacingY + m_markerSize / 2.0f,
                                            -zSize / 2.0f + float(z) * m_tickSpacingZ);

                currentMesh()->color4f(m_tickColor[0], m_tickColor[1], m_tickColor[2], m_tickColor[3]);

                if (z == 0)
                    currentMesh()->vertex3f(-xSize / 2.0f + (float)x * m_tickSpacingX,
                                            -ySize / 2.0f + (float)y * m_tickSpacingY,
                                            -zSize / 2.0f + float(z) * m_tickSpacingZ);
                else
                    currentMesh()->vertex3f(-xSize / 2.0f + (float)x * m_tickSpacingX,
                                            -ySize / 2.0f + (float)y * m_tickSpacingY,
                                            -zSize / 2.0f + float(z) * m_tickSpacingZ - m_markerSize / 2.0f);

                currentMesh()->color4f(m_tickColor[0], m_tickColor[1], m_tickColor[2], m_tickColor[3]);

                if (z == m_tickZ - 1)
                    currentMesh()->vertex3f(-xSize / 2.0f + (float)x * m_tickSpacingX,
                                            -ySize / 2.0f + (float)y * m_tickSpacingY,
                                            -zSize / 2.0f + float(z) * m_tickSpacingZ);
                else
                    currentMesh()->vertex3f(-xSize / 2.0f + (float)x * m_tickSpacingX,
                                            -ySize / 2.0f + (float)y * m_tickSpacingY,
                                            -zSize / 2.0f + float(z) * m_tickSpacingZ + m_markerSize / 2.0f);

                currentMesh()->color4f(m_tickColor[0], m_tickColor[1], m_tickColor[2], m_tickColor[3]);
            }
    currentMesh()->end();

    for (auto &mesh : meshes())
        mesh->setEnabled(false);

    mesh(2)->setLineWidth(2.0f);

    switch (m_gridType)
    {
    case GridType::Lines:
        mesh(1)->setEnabled(true);
        break;
    case GridType::Points:
        mesh(0)->setEnabled(true);
        break;
    case GridType::Markers:
        mesh(2)->setEnabled(true);
        break;
    case GridType::LinesAndMarkers:
        mesh(1)->setEnabled(true);
        mesh(2)->setEnabled(true);
        mesh(2)->setDepthFunc(GL_LEQUAL);
        break;
    default:
        mesh(2)->setEnabled(true);
    }
}

void Grid::setTicks(GLint nx, GLint ny, GLint nz)
{
    m_tickX = nx;
    m_tickY = ny;
    m_tickZ = nz;
    this->refresh();
}

GLint Grid::tickCountX()
{
    return m_tickX;
}

GLint Grid::tickCountY()
{
    return m_tickY;
}

GLint Grid::tickCountZ()
{
    return m_tickZ;
}

void Grid::setSpacing(GLfloat dx, GLfloat dy, GLfloat dz)
{
    m_tickSpacingX = dx;
    m_tickSpacingY = dy;
    m_tickSpacingZ = dz;
    this->refresh();
}

GLfloat Grid::tickSpacingX()
{
    return m_tickSpacingX;
}

GLfloat Grid::tickSpacingY()
{
    return m_tickSpacingY;
}

GLfloat Grid::tickSpacingZ()
{
    return m_tickSpacingZ;
}

void ivf::Grid::setType(GridType gridType)
{
    m_gridType = gridType;
    this->refresh();
}

GridType ivf::Grid::type()
{
    return m_gridType;
}

void ivf::Grid::setColor(float r, float g, float b, float a)
{
    m_tickColor[0] = r;
    m_tickColor[1] = g;
    m_tickColor[2] = b;
    m_tickColor[3] = a;
    this->refresh();
}

void ivf::Grid::setColor(glm::vec4 color)
{
    m_tickColor[0] = color.r;
    m_tickColor[1] = color.g;
    m_tickColor[2] = color.b;
    m_tickColor[3] = color.a;
    this->refresh();
}

void ivf::Grid::setColor(glm::vec3 color)
{
    m_tickColor[0] = color.r;
    m_tickColor[1] = color.g;
    m_tickColor[2] = color.b;
    m_tickColor[3] = 1.0;
    this->refresh();
}

void ivf::Grid::setColor(float r, float g, float b)
{
    m_tickColor[0] = r;
    m_tickColor[1] = g;
    m_tickColor[2] = b;
    m_tickColor[3] = 1.0;
    this->refresh();
}

glm::vec4 ivf::Grid::color()
{
    return glm::vec4(m_tickColor[0], m_tickColor[1], m_tickColor[2], m_tickColor[3]);
}

glm::vec3 ivf::Grid::color3()
{
    return glm::vec3(m_tickColor[0], m_tickColor[1], m_tickColor[2]);
}

void ivf::Grid::setLineColor(float r, float g, float b, float a)
{
    m_lineColor[0] = r;
    m_lineColor[1] = g;
    m_lineColor[2] = b;
    m_lineColor[3] = a;
    this->refresh();
}

void ivf::Grid::setLineColor(glm::vec4 color)
{
    m_lineColor[0] = color.r;
    m_lineColor[1] = color.g;
    m_lineColor[2] = color.b;
    m_lineColor[3] = color.a;
    this->refresh();
}

void ivf::Grid::setLineColor(glm::vec3 color)
{
    m_lineColor[0] = color.r;
    m_lineColor[1] = color.g;
    m_lineColor[2] = color.b;
    m_lineColor[3] = 1.0;
    this->refresh();
}

void ivf::Grid::setLineColor(float r, float g, float b)
{
    m_lineColor[0] = r;
    m_lineColor[1] = g;
    m_lineColor[2] = b;
    m_lineColor[3] = 1.0;
    this->refresh();
}

glm::vec4 ivf::Grid::lineColor()
{
    return glm::vec4(m_lineColor[0], m_lineColor[1], m_lineColor[2], m_lineColor[3]);
}

glm::vec3 ivf::Grid::lineColor3()
{
    return glm::vec3(m_lineColor[0], m_lineColor[1], m_lineColor[2]);
}

void ivf::Grid::setMarkerSize(float size)
{
    m_markerSize = size;
    this->refresh();
}

float ivf::Grid::markerSize()
{
    return m_markerSize;
}

void Grid::doPreDraw()
{
    LightManager::instance()->saveState();
    LightManager::instance()->disableLighting();

    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);
}

void Grid::doPostDraw()
{
    LightManager::instance()->restoreState();
    glDisable(GL_BLEND);
}
