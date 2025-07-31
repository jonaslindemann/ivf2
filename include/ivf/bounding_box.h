#pragma once

#include <glm/glm.hpp>
#include <vector>

namespace ivf {

/**
 * @class BoundingBox
 * @brief Represents an axis-aligned 3D bounding box with enhanced functionality.
 *
 * The BoundingBox class provides functionality to define, expand, and query an axis-aligned bounding box
 * in 3D space. It is commonly used for spatial queries, collision detection, and scene management.
 * Enhanced with transform support and additional utility methods.
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
     * @brief Constructor with explicit min/max values.
     * @param min Minimum coordinates.
     * @param max Maximum coordinates.
     */
    BoundingBox(const glm::vec3& min, const glm::vec3& max);

    /**
     * @brief Expand the bounding box to include the given position.
     * @param pos The position to include in the bounding box.
     */
    void add(glm::vec3 pos);

    /**
     * @brief Expand the bounding box to include another bounding box.
     * @param other The bounding box to include.
     */
    void add(const BoundingBox& other);

    /**
     * @brief Transform this bounding box by a transformation matrix.
     * @param transform The transformation matrix to apply.
     * @return BoundingBox The transformed bounding box.
     */
    BoundingBox transform(const glm::mat4& transform) const;

    /**
     * @brief Get the minimum (corner) coordinates of the bounding box.
     * @return glm::vec3 The minimum coordinates.
     */
    glm::vec3 min() const;

    /**
     * @brief Get the maximum (corner) coordinates of the bounding box.
     * @return glm::vec3 The maximum coordinates.
     */
    glm::vec3 max() const;

    /**
     * @brief Get the center point of the bounding box.
     * @return glm::vec3 The center coordinates.
     */
    glm::vec3 center() const;

    /**
     * @brief Get the size (width, height, depth) of the bounding box.
     * @return glm::vec3 The size along each axis.
     */
    glm::vec3 size() const;

    /**
     * @brief Get the extent (half-size) of the bounding box.
     * @return glm::vec3 The extent along each axis.
     */
    glm::vec3 extent() const;

    /**
     * @brief Check if the bounding box is valid (min <= max).
     * @return bool True if valid, false if empty/invalid.
     */
    bool isValid() const;

    /**
     * @brief Check if the bounding box is empty.
     * @return bool True if empty.
     */
    bool isEmpty() const;

    /**
     * @brief Check if this bounding box contains a point.
     * @param point The point to test.
     * @return bool True if the point is inside the bounding box.
     */
    bool contains(const glm::vec3& point) const;

    /**
     * @brief Check if this bounding box intersects with another.
     * @param other The other bounding box.
     * @return bool True if they intersect.
     */
    bool intersects(const BoundingBox& other) const;

    /**
     * @brief Get all 8 corner points of the bounding box.
     * @return std::vector<glm::vec3> Array of 8 corner points.
     */
    std::vector<glm::vec3> corners() const;

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

    /**
     * @brief Set both min and max coordinates.
     * @param min The new minimum coordinates.
     * @param max The new maximum coordinates.
     */
    void setBounds(const glm::vec3& min, const glm::vec3& max);
};

} // namespace ivf
