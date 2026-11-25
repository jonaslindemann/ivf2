#include <ivfvis/point_set.h>

using namespace ivfvis;

PointSet::PointSet(std::vector<glm::vec3> positions) : m_positions(std::move(positions))
{
}

std::shared_ptr<PointSet> PointSet::create()
{
    return std::make_shared<PointSet>();
}

std::shared_ptr<PointSet> PointSet::create(std::vector<glm::vec3> positions)
{
    return std::make_shared<PointSet>(std::move(positions));
}

std::size_t PointSet::pointCount() const
{
    return m_positions.size();
}

glm::vec3 PointSet::position(std::size_t index) const
{
    return m_positions[index];
}

std::span<glm::vec3> PointSet::positions()
{
    return m_positions;
}

std::span<const glm::vec3> PointSet::positions() const
{
    return m_positions;
}

void PointSet::setPositions(std::vector<glm::vec3> positions)
{
    m_positions = std::move(positions);
    resizeArrays();
}

std::size_t PointSet::addPoint(const glm::vec3 &position)
{
    m_positions.push_back(position);
    resizeArrays();
    return m_positions.size() - 1;
}

ivf::BoundingBox PointSet::bounds() const
{
    ivf::BoundingBox box;
    for (const auto &p : m_positions)
        box.add(p);
    return box;
}
