#pragma once

#include <ivf/nodes.h>

namespace ivf {

/**
 * @class GridLayout
 * @brief Utility for arranging nodes in a 3D grid layout.
 *
 * The GridLayout class provides methods to arrange nodes in a regular 3D grid pattern.
 * It supports configuration of grid size (rows, columns, layers) and spacing along each axis.
 * The layout can be applied to a CompositeNode, positioning its children in a grid.
 */
class GridLayout {
private:
    size_t m_rows;   ///< Number of rows in the grid.
    size_t m_cols;   ///< Number of columns in the grid.
    size_t m_layers; ///< Number of layers (depth) in the grid.

    float m_xSpacing; ///< Spacing between nodes along the X axis.
    float m_ySpacing; ///< Spacing between nodes along the Y axis.
    float m_zSpacing; ///< Spacing between nodes along the Z axis.

public:
    /**
     * @brief Constructor.
     * @param rows Number of rows.
     * @param cols Number of columns.
     * @param layers Number of layers (depth).
     * @param xSpacing Spacing along X axis.
     * @param ySpacing Spacing along Y axis.
     * @param zSpacing Spacing along Z axis.
     */
    GridLayout(size_t rows, size_t cols, size_t layers, float xSpacing, float ySpacing, float zSpacing);

    /**
     * @brief Destructor.
     */
    ~GridLayout();

    /**
     * @brief Set the spacing along each axis.
     * @param xSpacing Spacing along X axis.
     * @param ySpacing Spacing along Y axis.
     * @param zSpacing Spacing along Z axis.
     */
    void setSpacing(float xSpacing, float ySpacing, float zSpacing);

    /**
     * @brief Set the same spacing for all axes.
     * @param spacing Spacing value for all axes.
     */
    void setSpacing(float spacing);

    /**
     * @brief Set the grid size (rows, columns, layers).
     * @param rows Number of rows.
     * @param cols Number of columns.
     * @param layers Number of layers.
     */
    void setSize(size_t rows, size_t cols, size_t layers);

    /**
     * @brief Set the number of rows.
     * @param rows Number of rows.
     */
    void setRows(size_t rows);

    /**
     * @brief Set the number of columns.
     * @param cols Number of columns.
     */
    void setCols(size_t cols);

    /**
     * @brief Set the number of layers.
     * @param layers Number of layers.
     */
    void setLayers(size_t layers);

    /**
     * @brief Get the number of rows.
     * @return size_t Number of rows.
     */
    size_t rows();

    /**
     * @brief Get the number of columns.
     * @return size_t Number of columns.
     */
    size_t cols();

    /**
     * @brief Get the number of layers.
     * @return size_t Number of layers.
     */
    size_t layers();

    /**
     * @brief Get the spacing along the X axis.
     * @return float Spacing along X.
     */
    float xSpacing();

    /**
     * @brief Get the spacing along the Y axis.
     * @return float Spacing along Y.
     */
    float ySpacing();

    /**
     * @brief Get the spacing along the Z axis.
     * @return float Spacing along Z.
     */
    float zSpacing();

    /**
     * @brief Get the total number of grid positions (rows * cols * layers).
     * @return size_t Total grid size.
     */
    size_t size();

    /**
     * @brief Apply the grid layout to a CompositeNode (raw pointer).
     * @param composite Pointer to the CompositeNode.
     */
    void apply(ivf::CompositeNode *composite);

    /**
     * @brief Apply the grid layout to a CompositeNode (shared pointer).
     * @param composite Shared pointer to the CompositeNode.
     */
    void apply(ivf::CompositeNodePtr &composite);
};

} // namespace ivf
