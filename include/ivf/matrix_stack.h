#pragma once

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <vector>

namespace ivf {

class MatrixStack {
private:
    std::vector<glm::mat4> m_stack;

public:
    MatrixStack();

    void push();
    void loadIdentity();
    void pop();
    void multiply(glm::mat4 m);
    void translate(glm::vec3 pos);
    void rotate(float angle, glm::vec3 axis);
    void scale(glm::vec3 scale);
    void clear();
    size_t size();
    glm::mat4 top();
};

} // namespace ivf
