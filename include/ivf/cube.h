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
class Cube : public MeshNode {
private:
    GLfloat m_size; ///< Size of the cube (length of each edge).
public:
    /**
     * @brief Constructor.
     * @param size Length of each edge of the cube (default is 1.0).
     */
    Cube(GLfloat size = 1.0);

    /**
     * @brief Destructor.
     */
    virtual ~Cube();

    /**
     * @brief Factory method to create a shared pointer to a Cube instance.
     * @return std::shared_ptr<Cube> New Cube instance.
     */
    static std::shared_ptr<Cube> create();

    /**
     * @brief Set the size of the cube.
     * @param size New length for each edge of the cube.
     */
    void setSize(GLfloat size);

    /**
     * @brief Get the current size of the cube.
     * @return GLfloat Length of each edge of the cube.
     */
    GLfloat size();

protected:
    /**
     * @brief Internal setup method for initializing the cube geometry.
     */
    virtual void doSetup();

    virtual void setupProperties() override;
    virtual void onPropertyChanged(const std::string &name) override;
};

/**
 * @typedef CubePtr
 * @brief Shared pointer type for Cube.
 */
typedef std::shared_ptr<Cube> CubePtr;
}; // namespace ivf
