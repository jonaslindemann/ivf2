#pragma once

#include <ivf/mesh_node.h>

namespace ivf {

/**
 * @class Cube
 * @brief Node representing a 3D cube mesh with configurable size.
 *
 * The Cube class provides a simple 3D cube geometry. The cube's size can be customized.
 * Inherits from MeshNode, allowing transformation and integration into a scene graph.
 */
class Extents : public MeshNode {
private:
    BoundingBox m_bbox; ///< Bounding box of the cube.
public:
    /**
     * @brief Constructor.
     * @param size Length of each edge of the cube (default is 1.0).
     */
    Extents(BoundingBox bbox);

    /**
     * @brief Factory method to create a shared pointer to a Extents instance.
     * @return std::shared_ptr<Extents> New Extents instance.
     */
    static std::shared_ptr<Extents> create(BoundingBox bbox);

    /**
     * @brief Set the size of the cube.
     * @param size New length for each edge of the cube.
     */
    void setBoundingBox(BoundingBox bbox);

    /**
     * @brief Get the current size of the cube.
     * @return GLfloat Length of each edge of the cube.
     */
    BoundingBox bbox();

protected:
    /**
     * @brief Internal setup method for initializing the cube geometry.
     */
    virtual void doSetup();

    virtual void setupProperties() override;
    virtual void onPropertyChanged(const std::string &name) override;

    virtual void doPreDraw() override;
    virtual void doPostDraw() override;
};

/**
 * @typedef CubePtr
 * @brief Shared pointer type for Cube.
 */
typedef std::shared_ptr<Extents> ExtentsPtr;
}; // namespace ivf
