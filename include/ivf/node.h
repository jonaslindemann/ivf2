#pragma once

#include <ivf/glbase.h>
#include <ivf/material.h>
#include <ivf/texture.h>
#include <ivf/node_visitor.h>

namespace ivf {

class Node : public GLBase {
private:
    std::shared_ptr<Material> m_material{nullptr};
    std::shared_ptr<Texture> m_texture{nullptr};
    bool m_useMaterial{true};
    bool m_useTexture{false};
    bool m_visible{true};
    uint32_t m_objectId{0};

public:
    void draw();
    void drawSelection();

    void setMaterial(std::shared_ptr<Material> material);
    std::shared_ptr<Material> material();

    void setTexture(std::shared_ptr<Texture> texture);
    std::shared_ptr<Texture> texture();

    void setUseMaterial(bool flag);
    bool useMaterial();

    void setUseTexture(bool flag);
    bool useTexture();

    void setVisible(bool flag);
    bool visible() const;

    void setObjectId(uint32_t objectId);
    uint32_t objectId() const;

    uint32_t enumerateIds(uint32_t startId);

    virtual void accept(NodeVisitor *visitor);

protected:
    virtual void doPreDraw();
    virtual void doDraw();
    virtual void doPostDraw();

    virtual void doDrawSelection();

    virtual uint32_t doEnumerateIds(uint32_t startId);
};

typedef std::shared_ptr<Node> NodePtr;

} // namespace ivf
