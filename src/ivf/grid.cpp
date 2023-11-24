#include <ivf/grid.h>

#include <ivf/light_manager.h>

#include <iostream>

using namespace std;
using namespace ivf;

Grid::Grid()
    :m_tickX(11),
     m_tickY(11),
     m_tickZ(11),
     m_tickSpacingX(1.0e-0),
     m_tickSpacingY(1.0e-0),
     m_tickSpacingZ(1.0e-0)

{
    this->newMesh(m_tickX * m_tickY * m_tickZ);
    this->doSetup();
}

Grid::~Grid()
{
    
}

std::shared_ptr<Grid> Grid::create()
{
    return std::make_shared<Grid>();
}

void Grid::doSetup()
{
    long x, y, z;
    
    mesh()->setSize(m_tickX*m_tickY*m_tickZ);
    
    GLfloat xSize = m_tickSpacingX * (m_tickX-1);
    GLfloat ySize = m_tickSpacingY * (m_tickY-1);
    GLfloat zSize = m_tickSpacingZ * (m_tickZ-1);
    
    mesh()->begin(GL_POINTS);
    for (x = 0; x<m_tickX; x++)
        for (y = 0; y<m_tickY; y++)
            for (z = 0; z<m_tickZ; z++)
            {
                mesh()->vertex3f(-xSize/2.0f + (float)x * m_tickSpacingX, -ySize/2.0f + (float)y * m_tickSpacingY, -zSize/2.0f + float(z) * m_tickSpacingZ);
                mesh()->color3f((float)rand()/(float)RAND_MAX, (float)rand()/(float)RAND_MAX, (float)rand()/(float)RAND_MAX);
            }    
    mesh()->end();
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

void Grid::doPreDraw()
{
	LightManager::instance()->saveState();
	LightManager::instance()->disableLighting();
}

void Grid::doPostDraw()
{
	LightManager::instance()->restoreState();
}
