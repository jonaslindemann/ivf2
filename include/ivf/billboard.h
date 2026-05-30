#pragma once

/**
 * @file billboard.h
 * @brief A textured quad that always faces the camera.
 *
 * @code
 * auto bb = Billboard::create(1.0f, 1.0f);
 * auto tex = Texture::create(); tex->load("sprite.png");
 * bb->setTexture(tex);
 * bb->setPosition({0, 1, 0});
 * scene->add(bb);
 * @endcode
 *
 * Billboard uses the camera's right/up vectors (from the view matrix) so
 * it is GPU-oriented per-frame with no CPU round-trip. Use setAxisLocked(true)
 * for cylindrical billboards (e.g. trees) that only rotate around Y.
 */

#include <ivf/node.h>
#include <ivf/texture.h>
#include <glm/glm.hpp>
#include <memory>

namespace ivf {

class Billboard : public Node {
public:
    Billboard(float width = 1.0f, float height = 1.0f);
    ~Billboard() override;

    static std::shared_ptr<Billboard> create(float width = 1.0f, float height = 1.0f);

    void setTexture(TexturePtr tex)        { m_texture  = tex; }
    void setColor(glm::vec4 color)         { m_color    = color; }
    void setPosition(glm::vec3 pos)        { m_pos      = pos; }
    void setSize(float w, float h)         { m_width = w; m_height = h; }
    void setAxisLocked(bool lockY)         { m_axisLocked = lockY; }
    void setUVRegion(int col, int row, int cols, int rows);

    glm::vec3 position() const { return m_pos; }

    void doSetup() override;
    void doDraw()  override;

private:
    float      m_width{1.0f}, m_height{1.0f};
    glm::vec3  m_pos{0, 0, 0};
    glm::vec4  m_color{1, 1, 1, 1};
    glm::vec2  m_uvMin{0, 0}, m_uvMax{1, 1};
    bool       m_axisLocked{false};
    TexturePtr m_texture;

    GLuint m_vao{0}, m_vbo{0}, m_program{0};
    bool   m_gpuReady{false};

    void initGPU();
};

using BillboardPtr = std::shared_ptr<Billboard>;

} // namespace ivf
