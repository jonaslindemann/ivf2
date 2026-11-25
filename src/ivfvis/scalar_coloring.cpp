#include <ivfvis/scalar_coloring.h>

#include <ivf/colors.h>
#include <ivf/material.h>
#include <ivf/pbr_material.h>
#include <ivf/vertices.h>

#include <algorithm>
#include <vector>

using namespace ivfvis;

void ivfvis::colorVertices(ivf::Mesh &mesh, std::span<const float> values, const Colormap &colormap)
{
    auto colors = mesh.colors();
    if (!colors)
        return;
    const GLuint n = std::min<GLuint>(mesh.vertexCount(), static_cast<GLuint>(values.size()));
    for (GLuint i = 0; i < n; ++i)
    {
        const glm::vec4 c = colormap.color(values[i]);
        colors->setColor(i, c.r, c.g, c.b, c.a);
    }
    mesh.updateColors();
}

void ivfvis::colorVertices(ivf::MeshNode &node, const std::function<float(const glm::vec3 &)> &value,
                           const Colormap &colormap)
{
    std::vector<float> values;
    for (auto &mesh : node.meshes())
    {
        auto vertices = mesh->vertices();
        if (!vertices)
            continue;
        const GLuint n = mesh->vertexCount();
        values.resize(n);
        for (GLuint i = 0; i < n; ++i)
            values[i] = value(vertices->vertex(i));
        colorVertices(*mesh, values, colormap);
    }
}

void ivfvis::colorVertices(ivf::MeshNode &node, const UniformGrid &grid, std::string_view scalars,
                           const Colormap &colormap, const glm::mat4 &meshToGrid)
{
    static_cast<void>(grid.scalars(scalars)); // throws std::out_of_range for a missing array
    colorVertices(
        node,
        [&](const glm::vec3 &p) { return grid.sample(scalars, glm::vec3(meshToGrid * glm::vec4(p, 1.0f))); },
        colormap);
}

void ivfvis::useVertexColors(ivf::Node &node, bool flag)
{
    auto material = node.material();
    if (!material)
    {
        material = ivf::Material::create();
        node.setMaterial(material);
    }
    material->setUseVertexColor(flag);
    if (auto pbr = std::dynamic_pointer_cast<ivf::PBRMaterial>(material))
        pbr->setUseVertexColors(flag);
}
