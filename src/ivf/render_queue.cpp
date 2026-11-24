#include <ivf/render_queue.h>

#include <ivf/mesh.h>
#include <ivf/node.h>
#include <ivf/shader_material.h>

#include <algorithm>
#include <cstdint>
#include <map>
#include <unordered_map>
#include <utility>

using namespace ivf;

namespace {

// Material actually used for the item: the mesh's own, the node's own, otherwise the inherited one
const Material *effectiveMaterial(const DrawItem &item)
{
    if (item.mesh != nullptr && item.mesh->material())
        return item.mesh->material().get();
    if (item.node != nullptr && item.node->material() && item.node->useMaterial())
        return item.node->material().get();
    return item.material.get();
}

// Program the item is drawn with: a ShaderMaterial's, otherwise the default (nullptr)
const void *programOf(const Material *material)
{
    if (auto *shaderMaterial = dynamic_cast<const ShaderMaterial *>(material))
        return shaderMaterial->program().get();
    return nullptr;
}

// Squared distance from the eye to the item (bounds centre, else its origin)
float distance2(const DrawItem &item, const glm::vec3 &eye)
{
    const glm::vec3 p = item.bounds.isValid() ? item.bounds.center() : glm::vec3(item.world[3]);
    const glm::vec3 d = p - eye;
    return glm::dot(d, d);
}

struct SortKey {
    RenderLayer layer;
    std::size_t program;  // order of first appearance in the scene (deterministic, unlike addresses)
    std::size_t material; // same for materials
    std::size_t copies;   // 0, or 1 + first-seen number of a mesh drawn several times by one node
                          // (InstanceNode copies): keeps them adjacent for instancing
    float depth;          // squared distance; negated for back-to-front
};

// Numbers objects in the order they are first seen
class FirstSeen {
public:
    std::size_t operator()(const void *object)
    {
        return m_ids.try_emplace(object, m_ids.size()).first->second;
    }

private:
    std::unordered_map<const void *, std::size_t> m_ids;
};

} // namespace

void RenderQueue::sortForView(const glm::vec3 &eye)
{
    // Precompute keys, then sort indices (stable: ties keep the scene order)
    std::vector<SortKey> keys(m_items.size());
    FirstSeen programIds, materialIds;

    // Meshes drawn more than once by the same node (copies made by InstanceNode)
    auto copyKey = [](const DrawItem &item) { return std::pair<const void *, const void *>(item.mesh, item.node); };
    std::map<std::pair<const void *, const void *>, std::size_t> copyCount;
    for (const DrawItem &item : m_items)
        if (item.layer == RenderLayer::Opaque && item.mesh != nullptr)
            ++copyCount[copyKey(item)];
    std::map<std::pair<const void *, const void *>, std::size_t> copyIds;

    for (std::size_t i = 0; i < m_items.size(); ++i)
    {
        const DrawItem &item = m_items[i];
        SortKey &key = keys[i];
        key.layer = item.layer;
        key.program = 0;
        key.material = 0;
        key.copies = 0;
        key.depth = 0.0f;
        if (item.layer == RenderLayer::Opaque)
        {
            const Material *material = effectiveMaterial(item);
            key.program = programIds(programOf(material));
            key.material = materialIds(material);
            if (item.mesh != nullptr && copyCount[copyKey(item)] > 1)
                key.copies = 1 + copyIds.try_emplace(copyKey(item), copyIds.size()).first->second;
            key.depth = distance2(item, eye); // front to back
        }
        else if (item.layer == RenderLayer::Transparent)
        {
            key.depth = -distance2(item, eye); // back to front
        }
    }

    std::vector<std::size_t> order(m_items.size());
    for (std::size_t i = 0; i < order.size(); ++i)
        order[i] = i;

    std::stable_sort(order.begin(), order.end(), [&keys](std::size_t a, std::size_t b) {
        const SortKey &ka = keys[a];
        const SortKey &kb = keys[b];
        if (ka.layer != kb.layer)
            return static_cast<std::uint8_t>(ka.layer) < static_cast<std::uint8_t>(kb.layer);
        if (ka.program != kb.program)
            return ka.program < kb.program;
        if (ka.material != kb.material)
            return ka.material < kb.material;
        if (ka.copies != kb.copies)
            return ka.copies < kb.copies;
        return ka.depth < kb.depth;
    });

    std::vector<DrawItem> sorted;
    sorted.reserve(m_items.size());
    for (std::size_t i : order)
        sorted.push_back(std::move(m_items[i]));
    m_items = std::move(sorted);
}
