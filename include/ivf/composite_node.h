#pragma once

#include <ivf/transform_node.h>

#include <vector>

namespace ivf {

class CompositeNode : public TransformNode {
private:
    std::vector<std::shared_ptr<Node>> m_nodes;
    bool m_singleObjectId{false};

public:
    // Existing methods...
    CompositeNode();

    static std::shared_ptr<CompositeNode> create();

    void add(std::shared_ptr<Node> node);
    std::vector<std::shared_ptr<Node>> nodes();
    void clear();
    void remove(std::shared_ptr<Node> node);
    NodePtr at(size_t index);
    size_t count();

    virtual void accept(NodeVisitor *visitor) override;

    // Iterator type aliases
    using iterator = std::vector<NodePtr>::iterator;
    using const_iterator = std::vector<NodePtr>::const_iterator;

    // Iterator methods for range-based for loops
    iterator begin()
    {
        return m_nodes.begin();
    }
    iterator end()
    {
        return m_nodes.end();
    }

    // Const iterator methods
    const_iterator begin() const
    {
        return m_nodes.begin();
    }
    const_iterator end() const
    {
        return m_nodes.end();
    }

    // Optional: cbegin/cend for explicit const iteration
    const_iterator cbegin() const
    {
        return m_nodes.cbegin();
    }
    const_iterator cend() const
    {
        return m_nodes.cend();
    }

protected:
    virtual void doDraw();
    virtual uint32_t doEnumerateIds(uint32_t startId);
};

typedef std::shared_ptr<CompositeNode> CompositeNodePtr;
} // namespace ivf
