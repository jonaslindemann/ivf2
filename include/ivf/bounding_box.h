#pragma once

#include <glm/glm.hpp>

namespace ivf {

class BoundingBox {
private:
    glm::vec3 m_min;
    glm::vec3 m_max;

public:
    BoundingBox();

    void add(glm::vec3 pos);

    glm::vec3 min();
    glm::vec3 max();
    glm::vec3 center();
    glm::vec3 size();

    void clear();
    void setMin(glm::vec3 min);
    void setMax(glm::vec3 max);
};

} // namespace ivf
