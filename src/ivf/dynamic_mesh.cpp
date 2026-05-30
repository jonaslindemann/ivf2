#include <ivf/dynamic_mesh.h>
#include <ivf/mesh.h>

#include <algorithm>
#include <cstring>

namespace ivf {

DynamicMesh::DynamicMesh()
{}

std::shared_ptr<DynamicMesh> DynamicMesh::create()
{
    return std::make_shared<DynamicMesh>();
}

// ---- Builder ---------------------------------------------------------------

void DynamicMesh::beginMesh(GLenum primType)
{
    m_primType = primType;
    m_buffer.clear();
    m_indexBuffer.clear();
    m_current = RawVertex{};
    m_building = true;
}

void DynamicMesh::vertex(glm::vec3 pos)
{
    m_current.pos = pos;
    m_buffer.push_back(m_current);
}

void DynamicMesh::normal(glm::vec3 n)
{
    m_current.normal = n;
}

void DynamicMesh::texCoord(glm::vec2 uv)
{
    m_current.uv = uv;
}

void DynamicMesh::color(glm::vec4 c)
{
    m_current.color = c;
}

void DynamicMesh::index(unsigned int i)
{
    m_indexBuffer.push_back(i);
}

void DynamicMesh::endMesh()
{
    if (!m_building) return;
    m_building = false;
    commitMesh();
}

void DynamicMesh::commitMesh()
{
    // Clear any existing meshes
    m_meshes.clear();

    int vcount = static_cast<int>(m_buffer.size());
    int icount = static_cast<int>(m_indexBuffer.size());
    if (vcount == 0) return;

    newMesh(vcount, icount, m_primType, GL_DYNAMIC_DRAW);
    auto m = lastMesh();
    m->setLineWidth(m_lineWidth);

    m->begin(m_primType);
    for (auto& rv : m_buffer) {
        m->normal3f(rv.normal);
        m->color4f(rv.color.r, rv.color.g, rv.color.b, rv.color.a);
        m->tex2f(rv.uv.s, rv.uv.t);
        m->vertex3f(rv.pos);
    }
    for (auto i : m_indexBuffer)
        m->index1i(i);
    m->end();
}

// ---- Streaming update ------------------------------------------------------

void DynamicMesh::beginUpdate()
{
    m_updatingPositions = true;
    m_updateIdx = 0;
}

void DynamicMesh::vertex(glm::vec3 pos, bool /*updateMode*/)
{
    if (m_updateIdx < static_cast<int>(m_buffer.size())) {
        m_buffer[m_updateIdx].pos = pos;
        ++m_updateIdx;
    }
}

void DynamicMesh::endUpdate()
{
    m_updatingPositions = false;
    if (m_meshes.empty()) {
        commitMesh();
        return;
    }
    auto m = m_meshes[0];
    // Write updated positions into the mesh's vertex store
    auto verts = m->vertices();
    for (int i = 0; i < static_cast<int>(m_buffer.size()) && i < m->vertPos(); ++i)
        verts->setVertex(i, m_buffer[i].pos.x, m_buffer[i].pos.y, m_buffer[i].pos.z);
    m->updateVertices();
}

void DynamicMesh::updatePositions(const std::vector<glm::vec3>& positions)
{
    int count = static_cast<int>(std::min(positions.size(), m_buffer.size()));
    for (int i = 0; i < count; ++i)
        m_buffer[i].pos = positions[i];

    if (m_meshes.empty()) { commitMesh(); return; }

    auto m = m_meshes[0];
    auto verts = m->vertices();
    for (int i = 0; i < count; ++i)
        verts->setVertex(i, positions[i].x, positions[i].y, positions[i].z);
    m->updateVertices();
}

// ---- Appearance ------------------------------------------------------------

void DynamicMesh::setPointSize(float size)
{
    m_pointSize = size;
}

void DynamicMesh::setLineWidth(float width)
{
    m_lineWidth = width;
    if (!m_meshes.empty())
        m_meshes[0]->setLineWidth(width);
}

int DynamicMesh::vertexCount() const
{
    return static_cast<int>(m_buffer.size());
}

} // namespace ivf
