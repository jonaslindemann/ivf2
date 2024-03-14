#pragma once

#include <ivf/glbase.h>
#include <ivf/material.h>
#include <ivf/texture.h>

namespace ivf {

class Node : public GLBase {
private:
    std::shared_ptr<Material> m_material{nullptr};
    std::shared_ptr<Texture> m_texture{nullptr};
    bool m_useMaterial{true};
    bool m_useTexture{false};
    bool m_visible{true};

public:
    void draw();

    void setMaterial(std::shared_ptr<Material> material);
    void setTexture(std::shared_ptr<Texture> texture);

    void setUseMaterial(bool flag);
    bool useMaterial();

    void setUseTexture(bool flag);
    bool useTexture();

    void setVisible(bool flag);
    bool visible() const;

protected:
    virtual void doPreDraw();
    virtual void doDraw();
    virtual void doPostDraw();
};

typedef std::shared_ptr<Node> NodePtr;

} // namespace ivf
