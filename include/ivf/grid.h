#pragma once

#include <ivf/mesh_node.h>

namespace ivf {

/**
 * @enum GridType
 * @brief Types of grid rendering modes.
 */
enum GridType {
    Lines,          ///< Render grid as lines.
    Points,         ///< Render grid as points.
    Markers,        ///< Render grid as markers.
    LinesAndMarkers ///< Render grid as both lines and markers.
};

/**
 * @class Grid
 * @brief Node representing a 3D grid with configurable ticks, spacing, colors, and rendering type.
 *
 * The Grid class provides a 3D grid for visualization, debugging, or reference in a scene.
 * It supports configuration of tick counts, spacing, colors, marker size, and rendering type.
 * Inherits from MeshNode, allowing transformation and integration into a scene graph.
 */
class Grid : public MeshNode {
private:
    GLint m_tickX;          ///< Number of ticks along the X axis.
    GLint m_tickY;          ///< Number of ticks along the Y axis.
    GLint m_tickZ;          ///< Number of ticks along the Z axis.
    GLfloat m_tickSpacingX; ///< Spacing between ticks along the X axis.
    GLfloat m_tickSpacingY; ///< Spacing between ticks along the Y axis.
    GLfloat m_tickSpacingZ; ///< Spacing between ticks along the Z axis.
    GLfloat m_tickColor[4]; ///< RGBA color for grid ticks/markers.
    GLfloat m_lineColor[4]; ///< RGBA color for grid lines.
    GLfloat m_markerSize;   ///< Size of the grid markers.
    GridType m_gridType;    ///< Rendering type of the grid.

public:
    /**
     * @brief Default constructor.
     */
    Grid();

    /**
     * @brief Destructor.
     */
    virtual ~Grid();

    /**
     * @brief Factory method to create a shared pointer to a Grid instance.
     * @return std::shared_ptr<Grid> New Grid instance.
     */
    static std::shared_ptr<Grid> create();

    /**
     * @brief Set the number of ticks along each axis.
     * @param nx Number of ticks along X.
     * @param ny Number of ticks along Y.
     * @param nz Number of ticks along Z.
     */
    void setTicks(GLint nx, GLint ny, GLint nz);

    /**
     * @brief Get the number of ticks along the X axis.
     * @return GLint Number of ticks along X.
     */
    GLint tickCountX();

    /**
     * @brief Get the number of ticks along the Y axis.
     * @return GLint Number of ticks along Y.
     */
    GLint tickCountY();

    /**
     * @brief Get the number of ticks along the Z axis.
     * @return GLint Number of ticks along Z.
     */
    GLint tickCountZ();

    /**
     * @brief Set the spacing between ticks along each axis.
     * @param dx Spacing along X.
     * @param dy Spacing along Y.
     * @param dz Spacing along Z.
     */
    void setSpacing(GLfloat dx, GLfloat dy, GLfloat dz);

    /**
     * @brief Get the spacing between ticks along the X axis.
     * @return GLfloat Spacing along X.
     */
    GLfloat tickSpacingX();

    /**
     * @brief Get the spacing between ticks along the Y axis.
     * @return GLfloat Spacing along Y.
     */
    GLfloat tickSpacingY();

    /**
     * @brief Get the spacing between ticks along the Z axis.
     * @return GLfloat Spacing along Z.
     */
    GLfloat tickSpacingZ();

    /**
     * @brief Set the grid rendering type.
     * @param gridType The rendering type (Lines, Points, Markers, LinesAndMarkers).
     */
    void setType(GridType gridType);

    /**
     * @brief Get the current grid rendering type.
     * @return GridType The rendering type.
     */
    GridType type();

    /**
     * @brief Set the color for grid ticks/markers.
     * @param r Red component.
     * @param g Green component.
     * @param b Blue component.
     * @param a Alpha component.
     */
    void setColor(float r, float g, float b, float a);

    /**
     * @brief Set the color for grid ticks/markers using a glm::vec4.
     * @param color RGBA color.
     */
    void setColor(glm::vec4 color);

    /**
     * @brief Set the color for grid ticks/markers using a glm::vec3 (alpha = 1).
     * @param color RGB color.
     */
    void setColor(glm::vec3 color);

    /**
     * @brief Set the color for grid ticks/markers (alpha = 1).
     * @param r Red component.
     * @param g Green component.
     * @param b Blue component.
     */
    void setColor(float r, float g, float b);

    /**
     * @brief Get the current color for grid ticks/markers as glm::vec4.
     * @return glm::vec4 RGBA color.
     */
    glm::vec4 color();

    /**
     * @brief Get the current color for grid ticks/markers as glm::vec3.
     * @return glm::vec3 RGB color.
     */
    glm::vec3 color3();

    /**
     * @brief Set the color for grid lines.
     * @param r Red component.
     * @param g Green component.
     * @param b Blue component.
     * @param a Alpha component.
     */
    void setLineColor(float r, float g, float b, float a);

    /**
     * @brief Set the color for grid lines using a glm::vec4.
     * @param color RGBA color.
     */
    void setLineColor(glm::vec4 color);

    /**
     * @brief Set the color for grid lines using a glm::vec3 (alpha = 1).
     * @param color RGB color.
     */
    void setLineColor(glm::vec3 color);

    /**
     * @brief Set the color for grid lines (alpha = 1).
     * @param r Red component.
     * @param g Green component.
     * @param b Blue component.
     */
    void setLineColor(float r, float g, float b);

    /**
     * @brief Get the current color for grid lines as glm::vec4.
     * @return glm::vec4 RGBA color.
     */
    glm::vec4 lineColor();

    /**
     * @brief Get the current color for grid lines as glm::vec3.
     * @return glm::vec3 RGB color.
     */
    glm::vec3 lineColor3();

    /**
     * @brief Set the size of the grid markers.
     * @param size Marker size.
     */
    void setMarkerSize(float size);

    /**
     * @brief Get the current marker size.
     * @return float Marker size.
     */
    float markerSize();

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
 * @typedef GridPtr
 * @brief Shared pointer type for Grid.
 */
typedef std::shared_ptr<Grid> GridPtr;
}; // namespace ivf
