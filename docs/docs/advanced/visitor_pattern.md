# Visitors

A visitor walks through every node in the scene graph and does something with each one: count them, collect the ones of a certain type, compute the scene's size, or change a property everywhere at once. It saves you from writing the same recursive loop over `CompositeNode` children again and again.

## Writing a visitor

Subclass `ivf::NodeVisitor` and implement `visit()`. Then pass it to the root of the part of the scene you want to process:

```cpp
#include <ivf/node_visitor.h>

class CountVisitor : public ivf::NodeVisitor {
public:
    int count{0};
    void visit(ivf::Node *node) override { ++count; }
};

CountVisitor counter;
scene()->accept(&counter);
std::cout << counter.count << " nodes\n";
```

`accept()` calls `visit()` on the node itself, and a `CompositeNode` then passes the visitor on to each of its children, so the whole tree below it is visited, parents before children.

### Acting on specific node types

`visit()` receives every node as a plain `Node*`. Use `dynamic_cast` to pick out the types you care about:

```cpp
class WireframeVisitor : public ivf::NodeVisitor {
public:
    bool wireframe{true};

    void visit(ivf::Node *node) override
    {
        if (auto mesh = dynamic_cast<ivf::MeshNode *>(node))
            mesh->setWireframe(wireframe);
    }
};
```

### Collecting nodes

To build a list of nodes, store the raw pointers. They stay valid as long as the nodes remain in the scene:

```cpp
class LabelFinder : public ivf::NodeVisitor {
public:
    std::vector<ivf::TextNode *> labels;
    void visit(ivf::Node *node) override
    {
        if (auto text = dynamic_cast<ivf::TextNode *>(node))
            labels.push_back(text);
    }
};
```

## Built-in visitors

| Visitor | Purpose |
|---------|---------|
| `ExtentVisitor` | Computes the bounding box of everything visited, taking transforms into account. `bbox()` returns the result; pass `true` to the constructor to include hidden nodes |
| `PositionVisitor` | Collects the world position of every node |
| `MapVisitor` | Builds a map from object IDs to nodes, as used by [selection](selection.md) |
| `PrintVisitor` | Prints the scene structure to the console, handy for debugging loaded models |

For example, to centre the camera on part of the scene:

```cpp
#include <ivf/extent_visitor.h>

ivf::ExtentVisitor extent;
m_group->accept(&extent);
auto box = extent.bbox();
cameraManipulator()->setCameraTarget(box.center());
```

## When to use a visitor

- **Use a visitor** for operations over a whole tree whose structure you don't know in advance, such as a loaded model, or for reusable tools such as "make everything wireframe".
- **Keep your own list** of `shared_ptr`s instead when you created the nodes yourself and update them every frame; it is faster and type-safe.

Don't add or remove nodes inside `visit()`, because that changes the child lists while they are being walked. Collect the nodes first and change the scene afterwards.

## Related

- [Nodes](../core/nodes.md) — the scene graph the visitor walks
- [Model Loading](model_loading.md)
