#pragma once

#include <ivf/deformer.h>
#include <ivf/mesh_node.h>

namespace ivf {

template <typename PrimitiveType> class DeformablePrimitive : public DeformableMeshNode {
private:
    std::unique_ptr<PrimitiveType> m_primitive;

public:
    template <typename... Args> DeformablePrimitive(Args &&...args)
    {
        m_primitive = std::make_unique<PrimitiveType>(std::forward<Args>(args)...);
        copyFromPrimitive();
    }

    static std::shared_ptr<DeformablePrimitive<PrimitiveType>> create()
    {
        return std::make_shared<DeformablePrimitive<PrimitiveType>>();
    }

    template <typename... Args> static std::shared_ptr<DeformablePrimitive<PrimitiveType>> create(Args &&...args)
    {
        return std::make_shared<DeformablePrimitive<PrimitiveType>>(std::forward<Args>(args)...);
    }

    // Access the underlying primitive for configuration
    PrimitiveType *primitive()
    {
        return m_primitive.get();
    }
    const PrimitiveType *primitive() const
    {
        return m_primitive.get();
    }

    // Forward common primitive methods
    void refresh()
    {
        m_primitive->refresh();
        copyFromPrimitive();
        storeOriginalVertices();
    }

private:
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
