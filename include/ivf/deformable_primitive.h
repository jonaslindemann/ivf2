#pragma once

#include <ivf/deformer.h>
#include <ivf/mesh_node.h>

namespace ivf {

/**
 * @class DeformablePrimitive
 * @brief Template for mesh primitives supporting deformation via DeformableMeshNode.
 *
 * The DeformablePrimitive template class wraps any mesh primitive type (e.g., Box, Sphere) and
 * provides deformation capabilities by inheriting from DeformableMeshNode. It manages the
 * underlying primitive and synchronizes its mesh data for deformation and animation.
 *
 * @tparam PrimitiveType The mesh primitive type to be wrapped and made deformable.
 */
template <typename PrimitiveType> class DeformablePrimitive : public DeformableMeshNode {
private:
    std::unique_ptr<PrimitiveType> m_primitive; ///< The underlying mesh primitive.

public:
    /**
     * @brief Constructor forwarding arguments to the primitive.
     * @tparam Args Argument types for the primitive constructor.
     * @param args Arguments to forward to the primitive constructor.
     */
    template <typename... Args> DeformablePrimitive(Args &&...args)
    {
        m_primitive = std::make_unique<PrimitiveType>(std::forward<Args>(args)...);
        copyFromPrimitive();
    }

    /**
     * @brief Factory method to create a shared pointer to a DeformablePrimitive instance.
     * @return std::shared_ptr<DeformablePrimitive<PrimitiveType>> New DeformablePrimitive instance.
     */
    static std::shared_ptr<DeformablePrimitive<PrimitiveType>> create()
    {
        return std::make_shared<DeformablePrimitive<PrimitiveType>>();
    }

    /**
     * @brief Factory method to create a shared pointer to a DeformablePrimitive instance with arguments.
     * @tparam Args Argument types for the primitive constructor.
     * @param args Arguments to forward to the primitive constructor.
     * @return std::shared_ptr<DeformablePrimitive<PrimitiveType>> New DeformablePrimitive instance.
     */
    template <typename... Args> static std::shared_ptr<DeformablePrimitive<PrimitiveType>> create(Args &&...args)
    {
        return std::make_shared<DeformablePrimitive<PrimitiveType>>(std::forward<Args>(args)...);
    }

    /**
     * @brief Access the underlying primitive for configuration.
     * @return PrimitiveType* Pointer to the primitive.
     */
    PrimitiveType *primitive()
    {
        return m_primitive.get();
    }

    /**
     * @brief Access the underlying primitive for configuration (const version).
     * @return const PrimitiveType* Const pointer to the primitive.
     */
    const PrimitiveType *primitive() const
    {
        return m_primitive.get();
    }

    /**
     * @brief Refresh the primitive and update the deformable mesh node.
     *
     * Calls refresh on the underlying primitive, copies its mesh data, and stores the original vertices
     * for deformation.
     */
    void refresh()
    {
        m_primitive->refresh();
        copyFromPrimitive();
        storeOriginalVertices();
    }

private:
    /**
     * @brief Copy meshes from the primitive to this deformable node.
     */
    void copyFromPrimitive()
    {
        // Copy meshes from the primitive to this deformable node
        this->clear();
        for (auto &mesh : m_primitive->meshes())
        {
            this->addMesh(mesh);
        }
    }
};

} // namespace ivf
