#pragma once

#include <glm/glm.hpp>

namespace ivfmath {

struct KeyframeData {
    float timestamp;
    glm::vec3 position;
    glm::vec3 rotation;
    glm::vec3 scale;
    float tension; // Per-keyframe tension value

    KeyframeData(float time = 0.0f, glm::vec3 pos = glm::vec3(0.0f), glm::vec3 rot = glm::vec3(0.0f),
                 glm::vec3 scale = glm::vec3(1.0f),
                 float tension = 0.5f); // Default tension
};

} // namespace ivfmath
