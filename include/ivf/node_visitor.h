#pragma once

#include <memory>
#include <map>
// #include <cstdint>

namespace ivf {

class Node;
class CompositeNode;

class NodeVisitor {
public:
    virtual void visit(Node *node) = 0;
};

class PrintVisitor : public NodeVisitor {
private:
public:
    virtual void visit(Node *node) override;
};

typedef std::map<unsigned int, Node *> NodeMap;

class MapVisitor : public NodeVisitor {
private:
    NodeMap m_nodeMap;

public:
    virtual void visit(Node *node) override;

    NodeMap takeMap();
    const NodeMap &getMap() const;
    NodeMap getMapCopy() const;
};

} // namespace ivf
