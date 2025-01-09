#pragma once

#include <map>

#include <ivf/glbase.h>
#include <ivf/node.h>
#include <ivf/composite_node.h>
#include <ivf/node_visitor.h>

namespace ivf {

class BufferSelection : public GLBase {
private:
    GLuint m_fbo{0};
    GLuint m_colorTexture{0};
    GLuint m_depthRenderBuffer;

    int m_width{0};
    int m_height{0};

    CompositeNodePtr m_scene;

    NodeMap m_nodeMap;

public:
    BufferSelection(CompositeNodePtr scene);
    virtual ~BufferSelection();

    static std::shared_ptr<BufferSelection> create(CompositeNodePtr scene);

    void initialize(int width, int height);
    void resize(int width, int height);
    void begin();
    unsigned int idAtPixel(int x, int y);
    Node *nodeFromId(unsigned int objectId);
    Node *nodeAtPixel(int x, int y);
    void end();
    void clear();
};

typedef std::shared_ptr<BufferSelection> BufferSelectionPtr;

}; // namespace ivf
