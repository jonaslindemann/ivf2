#pragma once

#include <ivf/gl.h>
#include <ivf/texture.h>

namespace ivf {

class FrameBuffer : public GLBase {
private:
    GLuint m_frameBuffer;
    GLuint m_multisampledFrameBuffer;

    GLuint m_depthBuffer;
    GLuint m_multisampledDepthBuffer;

    GLuint m_colorTexture;
    GLuint m_multisampledColorTexture;

    GLuint m_quadVAO;
    GLuint m_quadVBO;

    bool m_multisample{false};
    int m_samples{4};

    int m_width;
    int m_height;

public:
    FrameBuffer(int width, int height);
    virtual ~FrameBuffer();

    static std::shared_ptr<FrameBuffer> create(int width, int height);

    void initQuad();

    void initialize();
    void resize(int width, int height);
    void drop();

    void bind();
    void unbind();

    void attachColorTexture(GLuint &texture);
    void attachDepthBuffer(GLuint &buffer);
    void attachMultisampledColorTexture();
    void attachMultisampledDepthBuffer();
    void attachMultisampledStencilBuffer();

    void setMultisample(bool multisample);
    void setSamples(int samples);

    bool multisample();
    int samples();

    void checkStatus();

    void begin();
    void end();

    void draw();

    GLuint colorTexture();

    GLuint id();
};

typedef std::shared_ptr<FrameBuffer> FrameBufferPtr;

}; // namespace ivf
