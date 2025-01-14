#pragma once

#include <ivf/mesh_node.h>

namespace ivf {

enum GridType {
    Lines,
    Points,
    Markers,
    LinesAndMarkers
};

class Grid : public MeshNode {
private:
    GLint m_tickX;
    GLint m_tickY;
    GLint m_tickZ;
    GLfloat m_tickSpacingX;
    GLfloat m_tickSpacingY;
    GLfloat m_tickSpacingZ;
    GLfloat m_tickColor[4];
    GLfloat m_lineColor[4];
    GLfloat m_markerSize;

    GridType m_gridType;

public:
    Grid();
    virtual ~Grid();

    static std::shared_ptr<Grid> create();

    void setTicks(GLint nx, GLint ny, GLint nz);
    GLint tickCountX();
    GLint tickCountY();
    GLint tickCountZ();

    void setSpacing(GLfloat dx, GLfloat dy, GLfloat dz);
    GLfloat tickSpacingX();
    GLfloat tickSpacingY();
    GLfloat tickSpacingZ();

    void setType(GridType gridType);
    GridType type();

    void setColor(float r, float g, float b, float a);
    void setColor(glm::vec4 color);
    void setColor(glm::vec3 color);
    void setColor(float r, float g, float b);

    glm::vec4 color();
    glm::vec3 color3();

    void setLineColor(float r, float g, float b, float a);
    void setLineColor(glm::vec4 color);
    void setLineColor(glm::vec3 color);
    void setLineColor(float r, float g, float b);

    glm::vec4 lineColor();
    glm::vec3 lineColor3();

    void setMarkerSize(float size);
    float markerSize();

protected:
    virtual void doSetup();
    virtual void doPreDraw();
    virtual void doPostDraw();
};

typedef std::shared_ptr<Grid> GridPtr;
}; // namespace ivf
