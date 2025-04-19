#pragma once

#include <ivf/glbase.h>
#include <vector>

#include <glm/glm.hpp>

namespace ivf {

class ShadowMap : public GLBase {
private:
    GLuint m_fbo{0};
    GLuint m_depthTexture{0};
    int m_width{1024};  // Default size
    int m_height{1024}; // Default size
    glm::mat4 m_lightSpaceMatrix;

public:
    ShadowMap(int width = 1024, int height = 1024);
    virtual ~ShadowMap();

    static std::shared_ptr<ShadowMap> create(int width = 1024, int height = 1024);

    void initialize();
    void resize(int width, int height);
    void bind();
    void unbind();

    GLuint depthTexture() const;
    void setLightSpaceMatrix(const glm::mat4 &matrix);
    glm::mat4 lightSpaceMatrix() const;
};

typedef std::shared_ptr<ShadowMap> ShadowMapPtr;

} // namespace ivf
