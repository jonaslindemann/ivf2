#pragma once

#include <ivf/mesh_node.h>

namespace ivf {

/**
 * @class Cursor
 * @brief Node representing a 3D cursor with axis lines and optional ground projection.
 *
 * The Cursor class provides a 3D cursor visualization consisting of three axis lines
 * (X, Y, Z) with customizable size and an optional line extending down to the XZ plane
 * to indicate position above ground level. The cursor supports configurable size,
 * gap between the Y-axis line and the ground projection line, and color customization.
 */
class Cursor : public MeshNode {
private:
    GLfloat m_size;                 ///< Length of each axis line.
    GLfloat m_gap;                  ///< Gap between Y-axis line and ground projection line.
    bool m_showGroundProjection;    ///< Whether to show the line extending to the XZ plane.
    bool m_useCustomColors;         ///< Whether to use custom colors for axis lines.
    GLfloat m_xAxisColor[4];        ///< RGBA color for X-axis line (default red).
    GLfloat m_yAxisColor[4];        ///< RGBA color for Y-axis line (default green).
    GLfloat m_zAxisColor[4];        ///< RGBA color for Z-axis line (default blue).
    GLfloat m_groundLineColor[4];   ///< RGBA color for ground projection line (default gray).

public:
    /**
     * @brief Constructor.
     * @param size Length of the axis lines (default is 1.0).
     * @param gap Gap between Y-axis line and ground projection (default is 0.1).
     * @param showGroundProjection Whether to show ground projection line (default is true).
     */
    Cursor(GLfloat size = 1.0, GLfloat gap = 0.1, bool showGroundProjection = true);

    /**
     * @brief Destructor.
     */
    virtual ~Cursor();

    /**
     * @brief Factory method to create a shared pointer to a Cursor instance.
     * @param size Length of the axis lines (default is 1.0).
     * @param gap Gap between Y-axis line and ground projection (default is 0.1).
     * @param showGroundProjection Whether to show ground projection line (default is true).
     * @return std::shared_ptr<Cursor> New Cursor instance.
     */
    static std::shared_ptr<Cursor> create(GLfloat size = 1.0, GLfloat gap = 0.1, bool showGroundProjection = true);

    /**
     * @brief Set the length of the cursor axis lines.
     * @param size New size for the cursor.
     */
    void setSize(GLfloat size);

    /**
     * @brief Get the current length of the cursor axis lines.
     * @return GLfloat The current cursor size.
     */
    GLfloat size() const;

    /**
     * @brief Set the gap between the Y-axis line and the ground projection line.
     * @param gap Gap distance (must be >= 0.0).
     */
    void setGap(GLfloat gap);

    /**
     * @brief Get the current gap between Y-axis and ground projection.
     * @return GLfloat The current gap distance.
     */
    GLfloat gap() const;

    /**
     * @brief Enable or disable the ground projection line.
     * @param show True to show the ground projection, false to hide it.
     */
    void setShowGroundProjection(bool show);

    /**
     * @brief Check if the ground projection line is shown.
     * @return bool True if ground projection is enabled.
     */
    bool showGroundProjection() const;

    /**
     * @brief Set custom colors for the axis lines.
     * @param xColor RGBA color for X-axis (red by default).
     * @param yColor RGBA color for Y-axis (green by default).
     * @param zColor RGBA color for Z-axis (blue by default).
     * @param groundColor RGBA color for ground projection (gray by default).
     */
    void setAxisColors(const GLfloat xColor[4], const GLfloat yColor[4], 
                      const GLfloat zColor[4], const GLfloat groundColor[4] = nullptr);

    /**
     * @brief Reset to default axis colors (X=red, Y=green, Z=blue, ground=gray).
     */
    void resetToDefaultColors();

    /**
     * @brief Set whether to use custom colors or default shader colors.
     * @param useCustom True to use custom colors, false for default.
     */
    void setUseCustomColors(bool useCustom);

    /**
     * @brief Check if custom colors are being used.
     * @return bool True if using custom colors.
     */
    bool useCustomColors() const;

protected:
    /**
     * @brief Internal setup method for initializing the cursor geometry.
     */
    virtual void doSetup() override;

    /**
     * @brief Called before drawing the cursor node.
     */
    virtual void doPreDraw() override;

    /**
     * @brief Called after drawing the cursor node.
     */
    virtual void doPostDraw() override;
};

/**
 * @typedef CursorPtr
 * @brief Shared pointer type for Cursor.
 */
typedef std::shared_ptr<Cursor> CursorPtr;

}; // namespace ivf