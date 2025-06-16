#pragma once

#include <ivf/base.h>
#include <ivf/mesh_node.h>

#include <memory>

namespace ivf {

/**
 * @class NormalFactory
 * @brief Utility class for generating or recalculating normals for mesh nodes.
 *
 * The NormalFactory class provides methods to generate or recalculate vertex normals
 * for MeshNode objects. This is useful for ensuring correct lighting and shading in
 * 3D scenes. The factory can be used to process a MeshNode and update its normals.
 */
class NormalFactory : public Base {
public:
    /**
     * @brief Default constructor.
     */
    NormalFactory();

    /**
     * @brief Factory method to create a shared pointer to a NormalFactory instance.
     * @return std::shared_ptr<NormalFactory> New NormalFactory instance.
     */
    static std::shared_ptr<NormalFactory> create();

    /**
     * @brief Generate or recalculate normals for the given mesh node.
     * @param meshNode Shared pointer to the MeshNode to process.
     * @return std::shared_ptr<MeshNode> The processed MeshNode with updated normals.
     */
    std::shared_ptr<MeshNode> create(std::shared_ptr<MeshNode> meshNode);
};

/**
 * @typedef NormalFactoryPtr
 * @brief Shared pointer type for NormalFactory.
 */
typedef std::shared_ptr<NormalFactory> NormalFactoryPtr;

} // namespace ivf
