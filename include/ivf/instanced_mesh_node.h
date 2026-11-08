#pragma once

/**
 * @file instanced_mesh_node.h
 * @brief Draws many copies of a mesh in a single draw call using GPU instancing.
 */

#include <ivf/mesh_node.h>
#include <ivf/transform_node.h>

#include <glm/glm.hpp>

#include <memory>
#include <utility>
#include <vector>

namespace ivf {

/**
 * @class InstancedMeshNode
 * @brief Renders the geometry of a MeshNode as many instances with one draw call per mesh.
 *
 * Each instance has its own transform, color and a free vec4 of data. The
 * transform is applied on top of the node's own transform. The geometry is
 * taken from a source MeshNode (any primitive or loaded model); the source
 * does not need to be part of the scene, and its own transform and material
 * are ignored. The node's material is used for all instances.
 *
 * The stock and PBR shaders support instancing: the instance color multiplies
 * the surface color. Custom shaders (ShaderMaterial) declare the per-instance
 * attributes themselves:
 * @code
 * layout (location = 4) in mat4 aInstanceModel; // locations 4-7
 * layout (location = 8) in vec4 aInstanceColor;
 * layout (location = 9) in vec4 aInstanceData;
 * uniform bool useInstancing;                    // true while an InstancedMeshNode draws
 * @endcode
 *
 * @code
 * auto flowers = InstancedMeshNode::create(Sphere::create(0.2));
 * for (int i = 0; i < 500; ++i)
 *     flowers->addInstance(InstancedMeshNode::makeTransform({x, 0, z}), color, glm::vec4(seed, 0, 0, 0));
 * flowers->setMaterial(material);
 * scene->add(flowers);
 * @endcode
 */
class InstancedMeshNode : public TransformNode {
private:
    struct InstanceAttributes {
        glm::mat4 transform{1.0f};
        glm::vec4 color{1.0f};
        glm::vec4 data{0.0f};
    };

    MeshNodePtr m_source;
    std::vector<InstanceAttributes> m_instances;

    GLuint m_instanceBuffer{0};
    size_t m_bufferCapacity{0}; ///< Allocated size of the instance buffer, in instances.
    bool m_dirty{true};

    /// Meshes whose vertex array has the instance attributes attached.
    std::vector<std::pair<std::weak_ptr<Mesh>, VertexArray *>> m_attachedMeshes;

    void uploadInstances();
    void attachInstanceAttributes(const std::shared_ptr<Mesh> &mesh);

public:
    /**
     * @brief Constructor.
     * @param source MeshNode providing the geometry.
     */
    InstancedMeshNode(MeshNodePtr source = nullptr);
    virtual ~InstancedMeshNode();

    /**
     * @brief Factory method.
     * @param source MeshNode providing the geometry.
     */
    static std::shared_ptr<InstancedMeshNode> create(MeshNodePtr source = nullptr);

    /**
     * @brief Build an instance transform from position, rotation and scale.
     * @param position Translation.
     * @param rotationDeg Euler angles in degrees, applied in x, y, z order.
     * @param scale Scale factors.
     */
    static glm::mat4 makeTransform(glm::vec3 position, glm::vec3 rotationDeg = glm::vec3(0.0f),
                                   glm::vec3 scale = glm::vec3(1.0f));

    void setSource(MeshNodePtr source);
    [[nodiscard]] MeshNodePtr source() const;

    /**
     * @brief Add an instance.
     * @param transform Instance transform, relative to the node.
     * @param color Instance color, multiplied with the surface color.
     * @param data Free per-instance values for custom shaders.
     * @return Index of the new instance.
     */
    size_t addInstance(const glm::mat4 &transform, glm::vec4 color = glm::vec4(1.0f),
                       glm::vec4 data = glm::vec4(0.0f));

    /**
     * @brief Set the number of instances. New instances get an identity
     * transform, white color and zero data.
     */
    void setInstanceCount(size_t count);
    [[nodiscard]] size_t instanceCount() const;
    void clearInstances();

    void setInstanceTransform(size_t index, const glm::mat4 &transform);
    void setInstanceColor(size_t index, glm::vec4 color);
    void setInstanceData(size_t index, glm::vec4 data);

    [[nodiscard]] glm::mat4 instanceTransform(size_t index) const;
    [[nodiscard]] glm::vec4 instanceColor(size_t index) const;
    [[nodiscard]] glm::vec4 instanceData(size_t index) const;

protected:
    virtual void doDraw() override;
};

using InstancedMeshNodePtr = std::shared_ptr<InstancedMeshNode>;

} // namespace ivf
