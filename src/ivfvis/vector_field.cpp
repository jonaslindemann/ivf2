#include <ivfvis/vector_field.h>

#include <utility>

using namespace ivfvis;

namespace {

bool inside(const ivf::BoundingBox &box, const glm::vec3 &p)
{
    if (!box.isValid())
        return true;
    const glm::vec3 lo = box.min(), hi = box.max();
    return p.x >= lo.x && p.y >= lo.y && p.z >= lo.z && p.x <= hi.x && p.y <= hi.y && p.z <= hi.z;
}

class GridField : public VectorField {
public:
    GridField(UniformGridPtr grid, std::string name) : m_grid(std::move(grid)), m_name(std::move(name))
    {
        static_cast<void>(m_grid->vectors(m_name)); // throws now, not on the first sample, if missing
    }
    glm::vec3 sample(const glm::vec3 &p) const override
    {
        return m_grid->sampleVector(m_name, p);
    }
    ivf::BoundingBox bounds() const override
    {
        return m_grid->bounds();
    }

private:
    UniformGridPtr m_grid;
    std::string m_name;
};

class FunctionField : public VectorField {
public:
    FunctionField(std::function<glm::vec3(const glm::vec3 &)> f, const ivf::BoundingBox &bounds)
        : m_f(std::move(f)), m_bounds(bounds)
    {
    }
    glm::vec3 sample(const glm::vec3 &p) const override
    {
        return m_f(p);
    }
    ivf::BoundingBox bounds() const override
    {
        return m_bounds;
    }

private:
    std::function<glm::vec3(const glm::vec3 &)> m_f;
    ivf::BoundingBox m_bounds;
};

class FlowFieldAdapter : public VectorField {
public:
    FlowFieldAdapter(ivf::FlowFieldPtr field, const ivf::BoundingBox &bounds)
        : m_field(std::move(field)), m_bounds(bounds)
    {
    }
    glm::vec3 sample(const glm::vec3 &p) const override
    {
        return m_field->sampleVelocity(p);
    }
    ivf::BoundingBox bounds() const override
    {
        return m_bounds;
    }

private:
    ivf::FlowFieldPtr m_field;
    ivf::BoundingBox m_bounds;
};

} // namespace

ivf::BoundingBox VectorField::bounds() const
{
    return ivf::BoundingBox();
}

bool VectorField::contains(const glm::vec3 &position) const
{
    return inside(bounds(), position);
}

VectorFieldPtr VectorField::fromGrid(UniformGridPtr grid, std::string name)
{
    return std::make_shared<GridField>(std::move(grid), std::move(name));
}

VectorFieldPtr VectorField::fromFunction(std::function<glm::vec3(const glm::vec3 &)> function,
                                         const ivf::BoundingBox &bounds)
{
    return std::make_shared<FunctionField>(std::move(function), bounds);
}

VectorFieldPtr VectorField::fromFlowField(ivf::FlowFieldPtr field, const ivf::BoundingBox &bounds)
{
    return std::make_shared<FlowFieldAdapter>(std::move(field), bounds);
}
