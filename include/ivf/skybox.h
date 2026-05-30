#pragma once

/**
 * @file skybox.h
 * @brief Renders a cubemap skybox behind everything in the scene.
 *
 * @code
 * auto cm = Cubemap::create();
 * cm->loadFromDirectory("textures/skybox");
 *
 * auto sky = Skybox::create(cm);
 * scene->add(sky);  // add before other nodes — it draws first with depth-write off
 * @endcode
 *
 * Skybox::doDraw() temporarily disables depth writes and uses a cube-map
 * projection that follows the camera but ignores translation.
 */

#include <ivf/node.h>
#include <ivf/cubemap.h>

namespace ivf {

class Skybox : public Node {
public:
    explicit Skybox(CubemapPtr cubemap = nullptr);
    ~Skybox() override;

    static std::shared_ptr<Skybox> create(CubemapPtr cubemap = nullptr);

    void setCubemap(CubemapPtr cubemap)    { m_cubemap  = cubemap; }
    void setExposure(float exposure)       { m_exposure = exposure; }
    CubemapPtr cubemap() const             { return m_cubemap; }

    void doSetup() override;
    void doDraw()  override;

private:
    CubemapPtr m_cubemap;
    float      m_exposure{1.0f};

    GLuint m_vao{0}, m_program{0};
    bool   m_gpuReady{false};

    void initGPU();
};

using SkyboxPtr = std::shared_ptr<Skybox>;

} // namespace ivf
