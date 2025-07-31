#include <ivf/extent_visitor.h>
#include <ivf/composite_node.h>

using namespace ivf;

// ExtentVisitor implementation

ExtentVisitor::ExtentVisitor(bool includeInvisible) 
    : m_matrixStack(), m_bbox(), m_includeInvisible(includeInvisible)
{}

void ExtentVisitor::visit(Node *node)
{
    if (!node || (!m_includeInvisible && !node->visible()))
        return;

    if (auto compositeNode = dynamic_cast<CompositeNode *>(node))
    {
        handleCompositeNode(compositeNode);
    }
    else if (auto transformNode = dynamic_cast<TransformNode *>(node))
    {
        handleTransformNode(transformNode);
    }
}

void ExtentVisitor::handleCompositeNode(CompositeNode *node)
{
    m_matrixStack.push();
    
    // Apply this node's transformation
    glm::mat4 localTransform = node->localTransform();
    m_matrixStack.multiply(localTransform);
    
    // Include this node's own bounding box if it has one
    BoundingBox localBbox = node->TransformNode::localBoundingBox(); // Get only this node's bbox, not children
    if (localBbox.isValid())
    {
        glm::mat4 currentTransform = m_matrixStack.top();
        BoundingBox transformedBbox = localBbox.transform(currentTransform);
        m_bbox.add(transformedBbox);
    }

    // Visit all children
    for (auto &child : node->nodes())
    {
        visit(child.get());
    }

    m_matrixStack.pop();
}

void ExtentVisitor::handleTransformNode(TransformNode *node)
{
    // Apply this node's transformation to the matrix stack
    m_matrixStack.push();
    glm::mat4 localTransform = node->localTransform();
    m_matrixStack.multiply(localTransform);
    
    // Get the node's local bounding box and transform it to world space
    BoundingBox localBbox = node->localBoundingBox();
    if (localBbox.isValid())
    {
        glm::mat4 currentTransform = m_matrixStack.top();
        BoundingBox transformedBbox = localBbox.transform(currentTransform);
        m_bbox.add(transformedBbox);
    }
    else
    {
        // Fallback: if no bounding box is set, just add the node's position
        glm::mat4 currentTransform = m_matrixStack.top();
        glm::vec4 transformedPos = currentTransform * glm::vec4(node->pos(), 1.0f);
        glm::vec3 worldPos = glm::vec3(transformedPos) / transformedPos.w;
        m_bbox.add(worldPos);
    }
    
    m_matrixStack.pop();
}

BoundingBox ExtentVisitor::bbox() const
{
    return m_bbox;
}

void ExtentVisitor::reset()
{
    m_bbox.clear();
    // Matrix stack should already be clear, but ensure it
    while (m_matrixStack.size() > 1)
        m_matrixStack.pop();
}

void ExtentVisitor::setIncludeInvisible(bool include)
{
    m_includeInvisible = include;
}

bool ExtentVisitor::includeInvisible() const
{
    return m_includeInvisible;
}

// PositionVisitor implementation

PositionVisitor::PositionVisitor(bool includeInvisible)
    : m_matrixStack(), m_positions(), m_includeInvisible(includeInvisible)
{}

void PositionVisitor::visit(Node *node)
{
    if (!node || (!m_includeInvisible && !node->visible()))
        return;

    if (auto compositeNode = dynamic_cast<CompositeNode *>(node))
    {
        handleCompositeNode(compositeNode);
    }
    else if (auto transformNode = dynamic_cast<TransformNode *>(node))
    {
        handleTransformNode(transformNode);
    }
}

void PositionVisitor::handleCompositeNode(CompositeNode *node)
{
    m_matrixStack.push();
    
    // Apply this node's transformation
    glm::mat4 localTransform = node->localTransform();
    m_matrixStack.multiply(localTransform);
    
    // Collect this node's position
    glm::mat4 currentTransform = m_matrixStack.top();
    glm::vec4 transformedPos = currentTransform * glm::vec4(node->pos(), 1.0f);
    glm::vec3 worldPos = glm::vec3(transformedPos) / transformedPos.w;
    m_positions.push_back(worldPos);

    // Visit all children
    for (auto &child : node->nodes())
    {
        visit(child.get());
    }

    m_matrixStack.pop();
}

void PositionVisitor::handleTransformNode(TransformNode *node)
{
    m_matrixStack.push();
    
    // Apply this node's transformation
    glm::mat4 localTransform = node->localTransform();
    m_matrixStack.multiply(localTransform);
    
    // Collect this node's world position
    glm::mat4 currentTransform = m_matrixStack.top();
    glm::vec4 transformedPos = currentTransform * glm::vec4(node->pos(), 1.0f);
    glm::vec3 worldPos = glm::vec3(transformedPos) / transformedPos.w;
    m_positions.push_back(worldPos);
    
    m_matrixStack.pop();
}

const std::vector<glm::vec3>& PositionVisitor::positions() const
{
    return m_positions;
}

void PositionVisitor::reset()
{
    m_positions.clear();
    // Matrix stack should already be clear, but ensure it
    while (m_matrixStack.size() > 1)
        m_matrixStack.pop();
}
