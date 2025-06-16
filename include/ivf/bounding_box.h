#pragma once

#include <glm/glm.hpp>

namespace ivf {

/**
 * @class BoundingBox
 * @brief Represents an axis-aligned 3D bounding box.
 *
 * The BoundingBox class provides functionality to define, expand, and query an axis-aligned bounding box
 * in 3D space. It is commonly used for spatial queries, collision detection, and scene management.
 */
class BoundingBox {
private:
    glm::vec3 m_min; ///< Minimum (corner) coordinates of the bounding box.
    glm::vec3 m_max; ///< Maximum (corner) coordinates of the bounding box.

public:
    /**
     * @brief Default constructor. Initializes the bounding box to an invalid or empty state.
     */
    BoundingBox();

    /**
     * @brief Expand the bounding box to include the given position.
     * @param pos The position to include in the bounding box.
     */
    void add(glm::vec3 pos);

    /**
     * @brief Get the minimum (corner) coordinates of the bounding box.
     * @return glm::vec3 The minimum coordinates.
     */
    glm::vec3 min();

    /**
     * @brief Get the maximum (corner) coordinates of the bounding box.
     * @return glm::vec3 The maximum coordinates.
     */
    glm::vec3 max();

    /**
     * @brief Get the center point of the bounding box.
     * @return glm::vec3 The center coordinates.
     */
    glm::vec3 center();

    /**
     * @brief Get the size (width, height, depth) of the bounding box.
     * @return glm::vec3 The size along each axis.
     */
    glm::vec3 size();

    /**
     * @brief Reset the bounding box to its initial (empty) state.
     */
    void clear();

    /**
     * @brief Set the minimum (corner) coordinates of the bounding box.
     * @param min The new minimum coordinates.
     */
    void setMin(glm::vec3 min);

    /**
     * @brief Set the maximum (corner) coordinates of the bounding box.
     * @param max The new maximum coordinates.
     */
    void setMax(glm::vec3 max);
};

} // namespace ivf
