#pragma once

#include <ivf/mesh_node.h>

namespace ivf {

enum class GridAlign {
    XY,
    XZ,
    YZ
};

class LineGrid : public MeshNode {
private:
    GLfloat m_width;
    GLfloat m_height;
    GLint m_xDivisions;
    GLint m_yDivisions;
    GridAlign m_align;

public:
    LineGrid(GLfloat width = 2.0, GLfloat height = 2.0, GLint xDiv = 10, GLint yDiv = 10);
    virtual ~LineGrid();

    static std::shared_ptr<LineGrid> create(GLfloat width = 2.0, GLfloat height = 2.0, GLint xDiv = 10,
                                            GLint yDiv = 10);

    void setSize(GLfloat width, GLfloat height);
    void setDivisions(GLint xDivisions, GLint yDivisions);

    GLfloat width() const;
    GLfloat height() const;
    GLint xDivisions() const;
    GLint yDivisions() const;
    void setAlign(GridAlign align);
    GridAlign align() const;

protected:
    virtual void doSetup();
    virtual void doPreDraw();
    virtual void doPostDraw();
};

typedef std::shared_ptr<LineGrid> LineGridPtr;

}; // namespace ivf
