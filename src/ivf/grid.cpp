#include <ivf/grid.h>

#include <ivf/light_manager.h>

#include <iostream>

using namespace std;
using namespace ivf;

Grid::Grid()
    : m_tickX(11), m_tickY(11), m_tickZ(11), m_tickSpacingX(1.0e-0), m_tickSpacingY(1.0e-0), m_tickSpacingZ(1.0e-0),
      m_gridType(GridType::Markers), m_tickColor{0.2, 0.2, 0.2, 1.0}, m_lineColor{0.1, 0.1, 0.1, 1.0}, m_markerSize(0.1),
      m_gridPlane(GridPlane::Full3D)
{
    this->doSetup();
    this->setName("Grid");
}

Grid::~Grid()
{}

std::shared_ptr<Grid> Grid::create()
{
    return std::make_shared<Grid>();
}

void Grid::setPlane(GridPlane plane)
{
    m_gridPlane = plane;
    this->refresh();
}

GridPlane Grid::plane() const
{
    return m_gridPlane;
}

void Grid::generatePointsMesh()
{
    long x, y, z;
    
    GLfloat xSize = m_tickSpacingX * (m_tickX - 1);
    GLfloat ySize = m_tickSpacingY * (m_tickY - 1);
    GLfloat zSize = m_tickSpacingZ * (m_tickZ - 1);

    switch (m_gridPlane) {
        case GridPlane::XY:
            // Grid in XY plane (Z = 0)
            newMesh(m_tickX * m_tickY);
            currentMesh()->begin(GL_POINTS);
            for (x = 0; x < m_tickX; x++) {
                for (y = 0; y < m_tickY; y++) {
                    currentMesh()->vertex3f(-xSize / 2.0f + (float)x * m_tickSpacingX,
                                          -ySize / 2.0f + (float)y * m_tickSpacingY,
                                          0.0f);
                    currentMesh()->color4f(m_tickColor[0], m_tickColor[1], m_tickColor[2], m_tickColor[3]);
                }
            }
            currentMesh()->end();
            break;
            
        case GridPlane::XZ:
            // Grid in XZ plane (Y = 0)
            newMesh(m_tickX * m_tickZ);
            currentMesh()->begin(GL_POINTS);
            for (x = 0; x < m_tickX; x++) {
                for (z = 0; z < m_tickZ; z++) {
                    currentMesh()->vertex3f(-xSize / 2.0f + (float)x * m_tickSpacingX,
                                          0.0f,
                                          -zSize / 2.0f + (float)z * m_tickSpacingZ);
                    currentMesh()->color4f(m_tickColor[0], m_tickColor[1], m_tickColor[2], m_tickColor[3]);
                }
            }
            currentMesh()->end();
            break;
            
        case GridPlane::YZ:
            // Grid in YZ plane (X = 0)
            newMesh(m_tickY * m_tickZ);
            currentMesh()->begin(GL_POINTS);
            for (y = 0; y < m_tickY; y++) {
                for (z = 0; z < m_tickZ; z++) {
                    currentMesh()->vertex3f(0.0f,
                                          -ySize / 2.0f + (float)y * m_tickSpacingY,
                                          -zSize / 2.0f + (float)z * m_tickSpacingZ);
                    currentMesh()->color4f(m_tickColor[0], m_tickColor[1], m_tickColor[2], m_tickColor[3]);
                }
            }
            currentMesh()->end();
            break;
            
        case GridPlane::Full3D:
        default:
            // Full 3D grid (original implementation)
            newMesh(m_tickX * m_tickY * m_tickZ);
            currentMesh()->begin(GL_POINTS);
            for (x = 0; x < m_tickX; x++) {
                for (y = 0; y < m_tickY; y++) {
                    for (z = 0; z < m_tickZ; z++) {
                        currentMesh()->vertex3f(-xSize / 2.0f + (float)x * m_tickSpacingX,
                                              -ySize / 2.0f + (float)y * m_tickSpacingY,
                                              -zSize / 2.0f + float(z) * m_tickSpacingZ);
                        currentMesh()->color4f(m_tickColor[0], m_tickColor[1], m_tickColor[2], m_tickColor[3]);
                    }
                }
            }
            currentMesh()->end();
            break;
    }
}

void Grid::generateLinesMesh()
{
    long x, y, z;
    
    GLfloat xSize = m_tickSpacingX * (m_tickX - 1);
    GLfloat ySize = m_tickSpacingY * (m_tickY - 1);
    GLfloat zSize = m_tickSpacingZ * (m_tickZ - 1);

    switch (m_gridPlane) {
        case GridPlane::XY:
            // Grid lines in XY plane
            newMesh(2 * (m_tickX + m_tickY));
            currentMesh()->begin(GL_LINES);
            
            // Vertical lines (X direction)
            for (x = 0; x < m_tickX; x++) {
                currentMesh()->vertex3f(-xSize / 2.0f + (float)x * m_tickSpacingX, -ySize / 2.0f, 0.0f);
                currentMesh()->color4f(m_lineColor[0], m_lineColor[1], m_lineColor[2], m_lineColor[3]);
                currentMesh()->vertex3f(-xSize / 2.0f + (float)x * m_tickSpacingX, ySize / 2.0f, 0.0f);
                currentMesh()->color4f(m_lineColor[0], m_lineColor[1], m_lineColor[2], m_lineColor[3]);
            }
            
            // Horizontal lines (Y direction)
            for (y = 0; y < m_tickY; y++) {
                currentMesh()->vertex3f(-xSize / 2.0f, -ySize / 2.0f + (float)y * m_tickSpacingY, 0.0f);
                currentMesh()->color4f(m_lineColor[0], m_lineColor[1], m_lineColor[2], m_lineColor[3]);
                currentMesh()->vertex3f(xSize / 2.0f, -ySize / 2.0f + (float)y * m_tickSpacingY, 0.0f);
                currentMesh()->color4f(m_lineColor[0], m_lineColor[1], m_lineColor[2], m_lineColor[3]);
            }
            currentMesh()->end();
            break;
            
        case GridPlane::XZ:
            // Grid lines in XZ plane
            newMesh(2 * (m_tickX + m_tickZ));
            currentMesh()->begin(GL_LINES);
            
            // Lines in X direction
            for (x = 0; x < m_tickX; x++) {
                currentMesh()->vertex3f(-xSize / 2.0f + (float)x * m_tickSpacingX, 0.0f, -zSize / 2.0f);
                currentMesh()->color4f(m_lineColor[0], m_lineColor[1], m_lineColor[2], m_lineColor[3]);
                currentMesh()->vertex3f(-xSize / 2.0f + (float)x * m_tickSpacingX, 0.0f, zSize / 2.0f);
                currentMesh()->color4f(m_lineColor[0], m_lineColor[1], m_lineColor[2], m_lineColor[3]);
            }
            
            // Lines in Z direction
            for (z = 0; z < m_tickZ; z++) {
                currentMesh()->vertex3f(-xSize / 2.0f, 0.0f, -zSize / 2.0f + (float)z * m_tickSpacingZ);
                currentMesh()->color4f(m_lineColor[0], m_lineColor[1], m_lineColor[2], m_lineColor[3]);
                currentMesh()->vertex3f(xSize / 2.0f, 0.0f, -zSize / 2.0f + (float)z * m_tickSpacingZ);
                currentMesh()->color4f(m_lineColor[0], m_lineColor[1], m_lineColor[2], m_lineColor[3]);
            }
            currentMesh()->end();
            break;
            
        case GridPlane::YZ:
            // Grid lines in YZ plane
            newMesh(2 * (m_tickY + m_tickZ));
            currentMesh()->begin(GL_LINES);
            
            // Lines in Y direction
            for (y = 0; y < m_tickY; y++) {
                currentMesh()->vertex3f(0.0f, -ySize / 2.0f + (float)y * m_tickSpacingY, -zSize / 2.0f);
                currentMesh()->color4f(m_lineColor[0], m_lineColor[1], m_lineColor[2], m_lineColor[3]);
                currentMesh()->vertex3f(0.0f, -ySize / 2.0f + (float)y * m_tickSpacingY, zSize / 2.0f);
                currentMesh()->color4f(m_lineColor[0], m_lineColor[1], m_lineColor[2], m_lineColor[3]);
            }
            
            // Lines in Z direction
            for (z = 0; z < m_tickZ; z++) {
                currentMesh()->vertex3f(0.0f, -ySize / 2.0f, -zSize / 2.0f + (float)z * m_tickSpacingZ);
                currentMesh()->color4f(m_lineColor[0], m_lineColor[1], m_lineColor[2], m_lineColor[3]);
                currentMesh()->vertex3f(0.0f, ySize / 2.0f, -zSize / 2.0f + (float)z * m_tickSpacingZ);
                currentMesh()->color4f(m_lineColor[0], m_lineColor[1], m_lineColor[2], m_lineColor[3]);
            }
            currentMesh()->end();
            break;
            
        case GridPlane::Full3D:
        default:
            // Original 3D lines implementation
            newMesh(2 * (m_tickX * m_tickY + m_tickX * m_tickZ + m_tickY * m_tickZ));
            currentMesh()->begin(GL_LINES);
            
            // XY plane lines (varying Z)
            for (x = 0; x < m_tickX; x++) {
                for (y = 0; y < m_tickY; y++) {
                    currentMesh()->vertex3f(-xSize / 2.0f + (float)x * m_tickSpacingX,
                                          -ySize / 2.0f + (float)y * m_tickSpacingY, -zSize / 2.0f);
                    currentMesh()->color4f(m_lineColor[0], m_lineColor[1], m_lineColor[2], m_lineColor[3]);
                    currentMesh()->vertex3f(-xSize / 2.0f + (float)x * m_tickSpacingX,
                                          -ySize / 2.0f + (float)y * m_tickSpacingY, zSize / 2.0f);
                    currentMesh()->color4f(m_lineColor[0], m_lineColor[1], m_lineColor[2], m_lineColor[3]);
                }
            }
            
            // XZ plane lines (varying Y)
            for (x = 0; x < m_tickX; x++) {
                for (z = 0; z < m_tickZ; z++) {
                    currentMesh()->vertex3f(-xSize / 2.0f + (float)x * m_tickSpacingX, -ySize / 2.0f,
                                          -zSize / 2.0f + float(z) * m_tickSpacingZ);
                    currentMesh()->color4f(m_lineColor[0], m_lineColor[1], m_lineColor[2], m_lineColor[3]);
                    currentMesh()->vertex3f(-xSize / 2.0f + (float)x * m_tickSpacingX, ySize / 2.0f,
                                          -zSize / 2.0f + float(z) * m_tickSpacingZ);
                    currentMesh()->color4f(m_lineColor[0], m_lineColor[1], m_lineColor[2], m_lineColor[3]);
                }
            }

            // YZ plane lines (varying X)
            for (y = 0; y < m_tickY; y++) {
                for (z = 0; z < m_tickZ; z++) {
                    currentMesh()->vertex3f(-xSize / 2.0f, -ySize / 2.0f + (float)y * m_tickSpacingY,
                                          -zSize / 2.0f + float(z) * m_tickSpacingZ);
                    currentMesh()->color4f(m_lineColor[0], m_lineColor[1], m_lineColor[2], m_lineColor[3]);
                    currentMesh()->vertex3f(xSize / 2.0f, -ySize / 2.0f + (float)y * m_tickSpacingY,
                                          -zSize / 2.0f + float(z) * m_tickSpacingZ);
                    currentMesh()->color4f(m_lineColor[0], m_lineColor[1], m_lineColor[2], m_lineColor[3]);
                }
            }
            currentMesh()->end();
            break;
    }
}

void Grid::generateMarkersMesh()
{
    long x, y, z;
    
    GLfloat xSize = m_tickSpacingX * (m_tickX - 1);
    GLfloat ySize = m_tickSpacingY * (m_tickY - 1);
    GLfloat zSize = m_tickSpacingZ * (m_tickZ - 1);

    switch (m_gridPlane) {
        case GridPlane::XY:
            // Markers in XY plane
            newMesh(m_tickX * m_tickY * 2 * 2); // 2 lines per marker, 2 directions
            currentMesh()->begin(GL_LINES);
            for (x = 0; x < m_tickX; x++) {
                for (y = 0; y < m_tickY; y++) {
                    float xPos = -xSize / 2.0f + (float)x * m_tickSpacingX;
                    float yPos = -ySize / 2.0f + (float)y * m_tickSpacingY;
                    float zPos = 0.0f;
                    
                    // X direction marker
                    currentMesh()->vertex3f(xPos - m_markerSize / 2.0f, yPos, zPos);
                    currentMesh()->color4f(m_tickColor[0], m_tickColor[1], m_tickColor[2], m_tickColor[3]);
                    currentMesh()->vertex3f(xPos + m_markerSize / 2.0f, yPos, zPos);
                    currentMesh()->color4f(m_tickColor[0], m_tickColor[1], m_tickColor[2], m_tickColor[3]);
                    
                    // Y direction marker
                    currentMesh()->vertex3f(xPos, yPos - m_markerSize / 2.0f, zPos);
                    currentMesh()->color4f(m_tickColor[0], m_tickColor[1], m_tickColor[2], m_tickColor[3]);
                    currentMesh()->vertex3f(xPos, yPos + m_markerSize / 2.0f, zPos);
                    currentMesh()->color4f(m_tickColor[0], m_tickColor[1], m_tickColor[2], m_tickColor[3]);
                }
            }
            currentMesh()->end();
            break;
            
        case GridPlane::XZ:
            // Markers in XZ plane
            newMesh(m_tickX * m_tickZ * 2 * 2); // 2 lines per marker, 2 directions
            currentMesh()->begin(GL_LINES);
            for (x = 0; x < m_tickX; x++) {
                for (z = 0; z < m_tickZ; z++) {
                    float xPos = -xSize / 2.0f + (float)x * m_tickSpacingX;
                    float yPos = 0.0f;
                    float zPos = -zSize / 2.0f + (float)z * m_tickSpacingZ;
                    
                    // X direction marker
                    currentMesh()->vertex3f(xPos - m_markerSize / 2.0f, yPos, zPos);
                    currentMesh()->color4f(m_tickColor[0], m_tickColor[1], m_tickColor[2], m_tickColor[3]);
                    currentMesh()->vertex3f(xPos + m_markerSize / 2.0f, yPos, zPos);
                    currentMesh()->color4f(m_tickColor[0], m_tickColor[1], m_tickColor[2], m_tickColor[3]);
                    
                    // Z direction marker
                    currentMesh()->vertex3f(xPos, yPos, zPos - m_markerSize / 2.0f);
                    currentMesh()->color4f(m_tickColor[0], m_tickColor[1], m_tickColor[2], m_tickColor[3]);
                    currentMesh()->vertex3f(xPos, yPos, zPos + m_markerSize / 2.0f);
                    currentMesh()->color4f(m_tickColor[0], m_tickColor[1], m_tickColor[2], m_tickColor[3]);
                }
            }
            currentMesh()->end();
            break;
            
        case GridPlane::YZ:
            // Markers in YZ plane
            newMesh(m_tickY * m_tickZ * 2 * 2); // 2 lines per marker, 2 directions
            currentMesh()->begin(GL_LINES);
            for (y = 0; y < m_tickY; y++) {
                for (z = 0; z < m_tickZ; z++) {
                    float xPos = 0.0f;
                    float yPos = -ySize / 2.0f + (float)y * m_tickSpacingY;
                    float zPos = -zSize / 2.0f + (float)z * m_tickSpacingZ;
                    
                    // Y direction marker
                    currentMesh()->vertex3f(xPos, yPos - m_markerSize / 2.0f, zPos);
                    currentMesh()->color4f(m_tickColor[0], m_tickColor[1], m_tickColor[2], m_tickColor[3]);
                    currentMesh()->vertex3f(xPos, yPos + m_markerSize / 2.0f, zPos);
                    currentMesh()->color4f(m_tickColor[0], m_tickColor[1], m_tickColor[2], m_tickColor[3]);
                    
                    // Z direction marker
                    currentMesh()->vertex3f(xPos, yPos, zPos - m_markerSize / 2.0f);
                    currentMesh()->color4f(m_tickColor[0], m_tickColor[1], m_tickColor[2], m_tickColor[3]);
                    currentMesh()->vertex3f(xPos, yPos, zPos + m_markerSize / 2.0f);
                    currentMesh()->color4f(m_tickColor[0], m_tickColor[1], m_tickColor[2], m_tickColor[3]);
                }
            }
            currentMesh()->end();
            break;
            
        case GridPlane::Full3D:
        default:
            // Original 3D markers implementation
            newMesh(m_tickX * m_tickY * m_tickZ * 2 * 3);
            currentMesh()->begin(GL_LINES);
            for (x = 0; x < m_tickX; x++) {
                for (y = 0; y < m_tickY; y++) {
                    for (z = 0; z < m_tickZ; z++) {
                        float xPos = -xSize / 2.0f + (float)x * m_tickSpacingX;
                        float yPos = -ySize / 2.0f + (float)y * m_tickSpacingY;
                        float zPos = -zSize / 2.0f + float(z) * m_tickSpacingZ;
                        
                        // X direction markers
                        if (x == 0)
                            currentMesh()->vertex3f(xPos, yPos, zPos);
                        else
                            currentMesh()->vertex3f(xPos - m_markerSize / 2.0f, yPos, zPos);
                        currentMesh()->color4f(m_tickColor[0], m_tickColor[1], m_tickColor[2], m_tickColor[3]);

                        if (x == m_tickX - 1)
                            currentMesh()->vertex3f(xPos, yPos, zPos);
                        else
                            currentMesh()->vertex3f(xPos + m_markerSize / 2.0f, yPos, zPos);
                        currentMesh()->color4f(m_tickColor[0], m_tickColor[1], m_tickColor[2], m_tickColor[3]);

                        // Y direction markers
                        if (y == 0)
                            currentMesh()->vertex3f(xPos, yPos, zPos);
                        else
                            currentMesh()->vertex3f(xPos, yPos - m_markerSize / 2.0f, zPos);
                        currentMesh()->color4f(m_tickColor[0], m_tickColor[1], m_tickColor[2], m_tickColor[3]);

                        if (y == m_tickY - 1)
                            currentMesh()->vertex3f(xPos, yPos, zPos);
                        else
                            currentMesh()->vertex3f(xPos, yPos + m_markerSize / 2.0f, zPos);
                        currentMesh()->color4f(m_tickColor[0], m_tickColor[1], m_tickColor[2], m_tickColor[3]);

                        // Z direction markers
                        if (z == 0)
                            currentMesh()->vertex3f(xPos, yPos, zPos);
                        else
                            currentMesh()->vertex3f(xPos, yPos, zPos - m_markerSize / 2.0f);
                        currentMesh()->color4f(m_tickColor[0], m_tickColor[1], m_tickColor[2], m_tickColor[3]);

                        if (z == m_tickZ - 1)
                            currentMesh()->vertex3f(xPos, yPos, zPos);
                        else
                            currentMesh()->vertex3f(xPos, yPos, zPos + m_markerSize / 2.0f);
                        currentMesh()->color4f(m_tickColor[0], m_tickColor[1], m_tickColor[2], m_tickColor[3]);
                    }
                }
            }
            currentMesh()->end();
            break;
    }
}

void Grid::doSetup()
{
    clear();

    // Generate the three different mesh types
    generatePointsMesh();  // mesh(0) - Points
    generateLinesMesh();   // mesh(1) - Lines  
    generateMarkersMesh(); // mesh(2) - Markers

    // Disable all meshes initially
    for (auto &mesh : meshes())
        mesh->setEnabled(false);

    // Set line width for markers
    if (meshes().size() > 2)
        mesh(2)->setLineWidth(2.0f);

    // Enable appropriate meshes based on grid type
    switch (m_gridType)
    {
    case GridType::Lines:
        if (meshes().size() > 1)
            mesh(1)->setEnabled(true);
        break;
    case GridType::Points:
        if (meshes().size() > 0)
            mesh(0)->setEnabled(true);
        break;
    case GridType::Markers:
        if (meshes().size() > 2)
            mesh(2)->setEnabled(true);
        break;
    case GridType::LinesAndMarkers:
        if (meshes().size() > 1)
            mesh(1)->setEnabled(true);
        if (meshes().size() > 2) {
            mesh(2)->setEnabled(true);
            mesh(2)->setDepthFunc(GL_LEQUAL);
        }
        break;
    default:
        if (meshes().size() > 2)
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
