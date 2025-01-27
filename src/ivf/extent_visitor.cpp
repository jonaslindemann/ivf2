#include <ivf/extent_visitor.h>

#include <ivf/composite_node.h>

using namespace ivf;

ExtentVisitor::ExtentVisitor() : m_matrixStack(), m_bbox()
{}

void ExtentVisitor::visit(Node *node)
{
    if (auto compositeNode = dynamic_cast<CompositeNode *>(node))
    {
        m_matrixStack.push();

        glm::mat4 transform = glm::translate(glm::mat4(1.0f), compositeNode->pos());
        m_matrixStack.multiply(transform);

        for (auto &child : compositeNode->nodes())
        {
            visit(child.get());
        }

        m_matrixStack.pop();
    }
    else if (auto transformNode = dynamic_cast<TransformNode *>(node))
    {
        handleTransformNode(transformNode);
    }
}

void ExtentVisitor::handleTransformNode(TransformNode *node)
{
    glm::mat4 currentTransform = m_matrixStack.top();

    glm::vec4 transformedVertex = currentTransform * glm::vec4(node->pos(), 1.0f);
    glm::vec3 worldPos = glm::vec3(transformedVertex) / transformedVertex.w;

    m_bbox.add(worldPos);
}

BoundingBox ExtentVisitor::bbox() const
{
    return m_bbox;
}
