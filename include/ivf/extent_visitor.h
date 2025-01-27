#pragma once

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <ivf/node_visitor.h>
#include <ivf/transform_node.h>
#include <ivf/matrix_stack.h>
#include <ivf/bounding_box.h>

namespace ivf {

class ExtentVisitor : public NodeVisitor {
    MatrixStack m_matrixStack;
    BoundingBox m_bbox;

public:
    ExtentVisitor();

    void visit(Node *node);

private:
    void handleTransformNode(TransformNode *node);

public:
    BoundingBox bbox() const;
};
} // namespace ivf
