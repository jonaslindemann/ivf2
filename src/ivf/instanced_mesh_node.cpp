#include <ivf/instanced_mesh_node.h>

#include <ivf/shader_manager.h>

#include <glm/gtc/matrix_transform.hpp>

#include <algorithm>
#include <cstddef>

using namespace ivf;

namespace {

// Attribute locations shared with the stock, PBR and custom shaders
constexpr GLuint kInstanceTransformLoc = 4; // mat4 occupies 4..7
constexpr GLuint kInstanceColorLoc = 8;
constexpr GLuint kInstanceDataLoc = 9;

} // namespace

InstancedMeshNode::InstancedMeshNode(MeshNodePtr source) : m_source(source)
{
    setName("InstancedMeshNode");
}

InstancedMeshNode::~InstancedMeshNode()
{
    if (m_instanceBuffer != 0)
        glDeleteBuffers(1, &m_instanceBuffer);
}

std::shared_ptr<InstancedMeshNode> InstancedMeshNode::create(MeshNodePtr source)
{
    return std::make_shared<InstancedMeshNode>(source);
}

glm::mat4 InstancedMeshNode::makeTransform(glm::vec3 position, glm::vec3 rotationDeg, glm::vec3 scale)
{
    glm::mat4 m = glm::translate(glm::mat4(1.0f), position);
    m = glm::rotate(m, glm::radians(rotationDeg.x), glm::vec3(1.0f, 0.0f, 0.0f));
    m = glm::rotate(m, glm::radians(rotationDeg.y), glm::vec3(0.0f, 1.0f, 0.0f));
    m = glm::rotate(m, glm::radians(rotationDeg.z), glm::vec3(0.0f, 0.0f, 1.0f));
    return glm::scale(m, scale);
}

void InstancedMeshNode::setSource(MeshNodePtr source)
{
    m_source = source;
    m_attachedMeshes.clear();
}

MeshNodePtr InstancedMeshNode::source() const
{
    return m_source;
}

size_t InstancedMeshNode::addInstance(const glm::mat4 &transform, glm::vec4 color, glm::vec4 data)
{
    m_instances.push_back({transform, color, data});
    m_dirty = true;
    return m_instances.size() - 1;
}

void InstancedMeshNode::setInstanceCount(size_t count)
{
    m_instances.resize(count);
    m_dirty = true;
}

size_t InstancedMeshNode::instanceCount() const
{
    return m_instances.size();
}

void InstancedMeshNode::clearInstances()
{
    m_instances.clear();
    m_dirty = true;
}

void InstancedMeshNode::setInstanceTransform(size_t index, const glm::mat4 &transform)
{
    if (index >= m_instances.size())
        return;
    m_instances[index].transform = transform;
    m_dirty = true;
}

void InstancedMeshNode::setInstanceColor(size_t index, glm::vec4 color)
{
    if (index >= m_instances.size())
        return;
    m_instances[index].color = color;
    m_dirty = true;
}

void InstancedMeshNode::setInstanceData(size_t index, glm::vec4 data)
{
    if (index >= m_instances.size())
        return;
    m_instances[index].data = data;
    m_dirty = true;
}

glm::mat4 InstancedMeshNode::instanceTransform(size_t index) const
{
    return index < m_instances.size() ? m_instances[index].transform : glm::mat4(1.0f);
}

glm::vec4 InstancedMeshNode::instanceColor(size_t index) const
{
    return index < m_instances.size() ? m_instances[index].color : glm::vec4(1.0f);
}

glm::vec4 InstancedMeshNode::instanceData(size_t index) const
{
    return index < m_instances.size() ? m_instances[index].data : glm::vec4(0.0f);
}

void InstancedMeshNode::uploadInstances()
{
    if (m_instanceBuffer == 0)
        glGenBuffers(1, &m_instanceBuffer);

    glBindBuffer(GL_ARRAY_BUFFER, m_instanceBuffer);

    const GLsizeiptr size = static_cast<GLsizeiptr>(m_instances.size() * sizeof(InstanceAttributes));
    if (m_instances.size() > m_bufferCapacity)
    {
        // Grow with headroom so adding instances one by one does not reallocate every frame
        m_bufferCapacity = std::max(m_instances.size(), m_bufferCapacity * 2);
        glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(m_bufferCapacity * sizeof(InstanceAttributes)),
                     nullptr, GL_DYNAMIC_DRAW);
    }
    glBufferSubData(GL_ARRAY_BUFFER, 0, size, m_instances.data());

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    m_dirty = false;
}

void InstancedMeshNode::attachInstanceAttributes(const std::shared_ptr<Mesh> &mesh)
{
    VertexArray *vao = mesh->vertexArray();
    if (vao == nullptr)
        return;

    for (const auto &[attachedMesh, attachedVao] : m_attachedMeshes)
        if (attachedMesh.lock() == mesh && attachedVao == vao)
            return;

    // Forget meshes that no longer exist (e.g. after the source was refreshed)
    std::erase_if(m_attachedMeshes, [](const auto &entry) { return entry.first.expired(); });

    vao->bind();
    glBindBuffer(GL_ARRAY_BUFFER, m_instanceBuffer);

    const GLsizei stride = sizeof(InstanceAttributes);
    for (GLuint col = 0; col < 4; ++col)
    {
        glEnableVertexAttribArray(kInstanceTransformLoc + col);
        glVertexAttribPointer(kInstanceTransformLoc + col, 4, GL_FLOAT, GL_FALSE, stride,
                              reinterpret_cast<void *>(offsetof(InstanceAttributes, transform) +
                                                       col * sizeof(glm::vec4)));
        glVertexAttribDivisor(kInstanceTransformLoc + col, 1);
    }

    glEnableVertexAttribArray(kInstanceColorLoc);
    glVertexAttribPointer(kInstanceColorLoc, 4, GL_FLOAT, GL_FALSE, stride,
                          reinterpret_cast<void *>(offsetof(InstanceAttributes, color)));
    glVertexAttribDivisor(kInstanceColorLoc, 1);

    glEnableVertexAttribArray(kInstanceDataLoc);
    glVertexAttribPointer(kInstanceDataLoc, 4, GL_FLOAT, GL_FALSE, stride,
                          reinterpret_cast<void *>(offsetof(InstanceAttributes, data)));
    glVertexAttribDivisor(kInstanceDataLoc, 1);

    vao->unbind();
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    m_attachedMeshes.emplace_back(mesh, vao);
}

void InstancedMeshNode::doDraw()
{
    if (!m_source || m_instances.empty())
        return;

    if (m_dirty || m_instanceBuffer == 0)
        uploadInstances();

    auto program = ShaderManager::instance()->currentProgram();
    program->uniformBool("useInstancing", true);

    const auto count = static_cast<GLsizei>(m_instances.size());
    for (auto &mesh : m_source->meshes())
    {
        if (!mesh->enabled())
            continue;
        attachInstanceAttributes(mesh);
        mesh->drawInstanced(count);
    }

    program->uniformBool("useInstancing", false);
}
