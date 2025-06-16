#pragma once

#include <ivf/mesh_node.h>

namespace ivf {

/**
 * @enum GridAlign
 * @brief Alignment options for the grid plane.
 */
enum class GridAlign {
    XY, ///< Grid lies in the XY plane.
    XZ, ///< Grid lies in the XZ plane.
    YZ  ///< Grid lies in the YZ plane.
};

/**
 * @class LineGrid
 * @brief Node representing a 2D grid of lines with configurable size, divisions, color, and alignment.
 *
 * The LineGrid class provides a 2D grid of lines for visualization or reference in a scene.
 * It supports configuration of grid size, number of divisions, color, and alignment to a principal plane.
 * Inherits from MeshNode, allowing transformation and integration into a scene graph.
 */
class LineGrid : public MeshNode {
private:
    GLfloat m_width;    ///< Width of the grid.
    GLfloat m_height;   ///< Height of the grid.
    GLint m_xDivisions; ///< Number of divisions along the X axis.
    GLint m_yDivisions; ///< Number of divisions along the Y axis.
    GridAlign m_align;  ///< Alignment of the grid plane.
    bool m_useColor;    ///< Whether to use a custom color.
    GLfloat m_color[4]; ///< RGBA color for the grid lines.

public:
    /**
     * @brief Constructor.
     * @param width Width of the grid.
     * @param height Height of the grid.
     * @param xDiv Number of divisions along X.
     * @param yDiv Number of divisions along Y.
     */
    LineGrid(GLfloat width = 2.0, GLfloat height = 2.0, GLint xDiv = 10, GLint yDiv = 10);

    /**
     * @brief Destructor.
     */
    virtual ~LineGrid();

    /**
     * @brief Factory method to create a shared pointer to a LineGrid instance.
     * @param width Width of the grid.
     * @param height Height of the grid.
     * @param xDiv Number of divisions along X.
     * @param yDiv Number of divisions along Y.
     * @return std::shared_ptr<LineGrid> New LineGrid instance.
     */
    static std::shared_ptr<LineGrid> create(GLfloat width = 2.0, GLfloat height = 2.0, GLint xDiv = 10,
                                            GLint yDiv = 10);

    /**
     * @brief Set the size of the grid.
     * @param width Width of the grid.
     * @param height Height of the grid.
     */
    void setSize(GLfloat width, GLfloat height);

    /**
     * @brief Set the number of divisions along each axis.
     * @param xDivisions Number of divisions along X.
     * @param yDivisions Number of divisions along Y.
     */
    void setDivisions(GLint xDivisions, GLint yDivisions);

    /**
     * @brief Set the color of the grid lines.
     * @param r Red component.
     * @param g Green component.
     * @param b Blue component.
     * @param a Alpha component (default 1.0).
     */
    void setColor(GLfloat r, GLfloat g, GLfloat b, GLfloat a = 1.0);

    /**
     * @brief Enable or disable the use of a custom color.
     * @param value True to use custom color, false to use default.
     */
    void setUseColor(bool value);

    /**
     * @brief Check if a custom color is used.
     * @return bool True if custom color is used.
     */
    bool useColor() const;

    /**
     * @brief Get the current color of the grid lines.
     * @param r Reference to store red component.
     * @param g Reference to store green component.
     * @param b Reference to store blue component.
     * @param a Reference to store alpha component.
     */
    void getColor(GLfloat &r, GLfloat &g, GLfloat &b, GLfloat &a);

    /**
     * @brief Get the width of the grid.
     * @return GLfloat Width of the grid.
     */
    GLfloat width() const;

    /**
     * @brief Get the height of the grid.
     * @return GLfloat Height of the grid.
     */
    GLfloat height() const;

    /**
     * @brief Get the number of divisions along the X axis.
     * @return GLint Number of divisions along X.
     */
    GLint xDivisions() const;

    /**
     * @brief Get the number of divisions along the Y axis.
     * @return GLint Number of divisions along Y.
     */
    GLint yDivisions() const;

    /**
     * @brief Set the alignment of the grid plane.
     * @param align Alignment option (XY, XZ, YZ).
     */
    void setAlign(GridAlign align);

    /**
     * @brief Get the current alignment of the grid plane.
     * @return GridAlign Alignment option.
     */
    GridAlign align() const;

protected:
    /**
     * @brief Internal setup method for initializing the grid geometry.
     */
    virtual void doSetup();

    /**
     * @brief Called before drawing the grid node.
     */
    virtual void doPreDraw();

    /**
     * @brief Called after drawing the grid node.
     */
    virtual void doPostDraw();
};

/**
 * @typedef LineGridPtr
 * @brief Shared pointer type for LineGrid.
 */
typedef std::shared_ptr<LineGrid> LineGridPtr;

}; // namespace ivf
