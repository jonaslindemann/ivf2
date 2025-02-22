#pragma once

#include <ivf/mesh_node.h>

namespace ivf {

class LineTrace : public MeshNode {
private:
    int m_numVertices;
    bool m_useColor;
    GLfloat m_color[4];
    bool m_firstAdd;

public:
    LineTrace(int numVertices);
    virtual ~LineTrace();

    static std::shared_ptr<LineTrace> create(int numVertices);

    void setColor(GLfloat r, GLfloat g, GLfloat b, GLfloat a = 1.0);
    void setUseColor(bool value);
    bool useColor() const;
    void getColor(GLfloat &r, GLfloat &g, GLfloat &b, GLfloat &a);

    void setNumVertices(int numVertices);
    int numVertices() const;

    void setVertex(int idx, glm::vec3 &vertex);

    void reset();
    void start(glm::vec3 vertex);
    void add(glm::vec3 vertex);
    void setSize(size_t size);

    void refresh();

protected:
    virtual void doSetup();
    virtual void doPreDraw();
    virtual void doPostDraw();
};

typedef std::shared_ptr<LineTrace> LineTracePtr;

}; // namespace ivf
