#include <ivfvis/data_set.h>

#include <stdexcept>

using namespace ivfvis;

namespace {

template <typename Map> auto &findArray(Map &map, std::string_view name, const char *kind)
{
    auto it = map.find(name);
    if (it == map.end())
        throw std::out_of_range(std::string("DataSet: no ") + kind + " array '" + std::string(name) + "'");
    return it->second;
}

} // namespace

std::span<float> DataSet::addScalars(std::string_view name, float value)
{
    auto &values = m_scalars[std::string(name)];
    values.assign(pointCount(), value);
    return values;
}

std::span<glm::vec3> DataSet::addVectors(std::string_view name, const glm::vec3 &value)
{
    auto &values = m_vectors[std::string(name)];
    values.assign(pointCount(), value);
    return values;
}

std::span<float> DataSet::fillScalars(std::string_view name, const std::function<float(const glm::vec3 &)> &f)
{
    auto values = addScalars(name);
    for (std::size_t i = 0; i < values.size(); ++i)
        values[i] = f(position(i));
    return values;
}

std::span<glm::vec3> DataSet::fillVectors(std::string_view name, const std::function<glm::vec3(const glm::vec3 &)> &f)
{
    auto values = addVectors(name);
    for (std::size_t i = 0; i < values.size(); ++i)
        values[i] = f(position(i));
    return values;
}

std::span<float> DataSet::addMagnitude(std::string_view vectorName, std::string_view scalarName)
{
    const auto &vectors = findArray(m_vectors, vectorName, "vector");
    auto &values = m_scalars[std::string(scalarName)];
    values.resize(vectors.size());
    for (std::size_t i = 0; i < vectors.size(); ++i)
        values[i] = glm::length(vectors[i]);
    return values;
}

bool DataSet::hasScalars(std::string_view name) const
{
    return m_scalars.find(name) != m_scalars.end();
}

bool DataSet::hasVectors(std::string_view name) const
{
    return m_vectors.find(name) != m_vectors.end();
}

std::span<float> DataSet::scalars(std::string_view name)
{
    return findArray(m_scalars, name, "scalar");
}

std::span<const float> DataSet::scalars(std::string_view name) const
{
    return findArray(m_scalars, name, "scalar");
}

std::span<glm::vec3> DataSet::vectors(std::string_view name)
{
    return findArray(m_vectors, name, "vector");
}

std::span<const glm::vec3> DataSet::vectors(std::string_view name) const
{
    return findArray(m_vectors, name, "vector");
}

void DataSet::removeArray(std::string_view name)
{
    if (auto it = m_scalars.find(name); it != m_scalars.end())
        m_scalars.erase(it);
    if (auto it = m_vectors.find(name); it != m_vectors.end())
        m_vectors.erase(it);
}

std::vector<std::string> DataSet::scalarNames() const
{
    std::vector<std::string> names;
    for (const auto &[name, values] : m_scalars)
        names.push_back(name);
    return names;
}

std::vector<std::string> DataSet::vectorNames() const
{
    std::vector<std::string> names;
    for (const auto &[name, values] : m_vectors)
        names.push_back(name);
    return names;
}

ValueRange DataSet::range(std::string_view name) const
{
    ValueRange range;
    if (auto it = m_scalars.find(name); it != m_scalars.end())
    {
        for (float v : it->second)
            range.add(v);
        return range;
    }
    for (const glm::vec3 &v : findArray(m_vectors, name, "scalar or vector"))
        range.add(glm::length(v));
    return range;
}

void DataSet::resizeArrays()
{
    const std::size_t n = pointCount();
    for (auto &[name, values] : m_scalars)
        values.resize(n, 0.0f);
    for (auto &[name, values] : m_vectors)
        values.resize(n, glm::vec3(0.0f));
}
